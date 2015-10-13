#include "planificador.h"

//falta optimizar con shared library sockets
void* iniciarServidor() {
	fd_set set_maestro, set_temporal, socketsHilos;
	uint32_t fdMaximo, socketProcesado, socketReceptor, nuevoFd;
	FD_ZERO(&set_maestro);	//Limpia el set maestro
	FD_ZERO(&set_temporal); //Limpia el set temporal
	FD_ZERO(&socketsHilos);
	struct sockaddr_storage remoteaddr; //dirección del cliente
	socklen_t addrlen;
	socketReceptor = crearSocketReceptor();
	listen(socketReceptor, 10);	// IMPORTANTE: listen() es una syscall BLOQUEANTE.
	FD_SET(socketReceptor, &set_maestro);	//Agrega receptor al set maestro
	fdMaximo = socketReceptor;	//Hacer seguimiento de descriptor maximo
	int32_t cabecera;
	uint32_t *hiloCreado = malloc(sizeof(uint32_t));
	*hiloCreado = 0;
	printf("Esperando conexiones\n");
	for (;;) {
		set_temporal = set_maestro;
		if (select(fdMaximo + 1, &set_temporal, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(EXIT_SUCCESS);
		}
		//Recorre las conexiones existentes en busca de datos para leer
		for (socketProcesado = 0; socketProcesado <= fdMaximo;
				socketProcesado++) {
			if (FD_ISSET(socketProcesado, &set_temporal)) //Si el socketProcesado pertenece al setTemporal
			{
				if (socketProcesado == socketReceptor) //Aca se recibe nueva conexion
				{
					addrlen = sizeof(remoteaddr);
					if ((nuevoFd = accept(socketReceptor,
							(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
						printf("Error en accept\n");
					} else {
						FD_SET(nuevoFd, &set_maestro);
						if (nuevoFd > fdMaximo) //Actualizo maximo descriptor de socket
						{
							fdMaximo = nuevoFd;
						}
					}
				} else //Aca se ejecuta el socket procesado
				{
					cabecera = deserializarEnteroSinSigno(socketProcesado);
					switch (cabecera) {
					case AGREGARPADRECPU:
						creoPadre(socketProcesado);
						break;
					case AGREGARHILOCPU:
						creoCpu(socketProcesado);
						generoHiloPlanificador(hiloCreado);
						break;
					case LOGEARRESULTADOCPU:
						logearResultadoCpu(socketProcesado);
						break;
					case BLOQUEOPCB:
						replanificar(socketProcesado);
						break;
					case LOGEARFINALIZACIONCPU:
						logearFinalizacionCpu(socketProcesado);
						break;
					}
					if (cabecera <= 0) {
						log_info(planificadorLog, "Se desconecto cpu con socketId: %d", socketProcesado);
						close(socketProcesado);
						FD_CLR(socketProcesado, &set_maestro);
					}
				}
			}
		}
	}
	return NULL;
}

int32_t deserializarEnteroSinSigno(uint32_t socket) {
	int32_t enteroSinSigno;
	int32_t status = recv(socket, &enteroSinSigno, sizeof(int32_t), 0);
	if (status == -1 || status == 0) {
		enteroSinSigno = status;
	}
	return enteroSinSigno;

}

void generoHiloPlanificador(uint32_t *hilo) {
	pthread_t hiloPlanificador;
	int respPlanificador = 0;
	if (!*hilo) {
		*hilo = 1;
		respPlanificador = pthread_create(&hiloPlanificador, NULL, (void *) consumirRecursos, NULL);
	}
	if (respPlanificador) {
		fprintf(stderr, "Error- Iniciar servidor codigo de retorno %d\n", respPlanificador);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}
}

void consumirRecursos() {
	//mientras halla cpu listos, mandar a ejecutar pcb
	while (1) {
		sem_wait(&sincrocpu);
		sem_wait(&sincroproc);
		while (list_size(cpu_listos) > 0 && list_size(proc_listos) > 0) {
			sem_wait(&mutex);
			t_list *cpu_ocupado = list_take_and_remove(cpu_listos, 1);
			t_list *proc_ejecutado = list_take_and_remove(proc_listos, 1);
			//Mandar por socket al cpu el proc para ejecutar

			t_pcb *pcb = list_get(proc_ejecutado, 0);
			t_hilosConectados *cpu = list_get(cpu_ocupado, 0);
			pcb->estadoProceso = 1; //Lo ponemos en estado de ejecucion
			cpu->estadoHilo = 1; // Lo ponemos en estado de ejecucion
			cpu->idProceso = pcb->idProceso; //Asigno al cpu su IdProceso que va a ejec.
			uint32_t *totalPaquete = malloc(sizeof(uint32_t));

			char* pcbSerializado = serializarPCB(pcb, totalPaquete);

			char* mensaje = malloc(*totalPaquete);
			memcpy(mensaje, pcbSerializado, *totalPaquete);
			int sendByte = send(cpu->socketHilo, mensaje, *totalPaquete, 0);
			if (sendByte < 0) {
				log_error(planificadorLog, "Error al enviar el proc/pcb al cpu",
						"ERROR");
			}
			//Asignar los cpu y proc usados en la lista de ocupados y ejecutados.
			list_add(proc_ejecutados, pcb);
			list_add(cpu_ocupados, cpu);
			log_info(planificadorLog, "Se envio a ejecutar el programa: %s con PID: %d", pcb->path, pcb->idProceso);
			free(mensaje);
			free(pcbSerializado);
			free(totalPaquete);
			sem_post(&mutex);
		}
	}
}

void logearResultadoCpu(uint32_t socketCpu) {
	char* mensajeCpu = recibirMensaje(socketCpu);
	log_info(planificadorLog, "Mensaje de cpu: %s", mensajeCpu);
	free(mensajeCpu);
}

char* recibirMensaje(uint32_t socket) {
	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje = 0;
	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	int32_t status = recv(socket, &longitudMensaje, sizeof(int32_t), 0);
	if(status<0)
		return "error";
	char* mensaje = (char*) malloc(longitudMensaje + 1);
	recv(socket, mensaje, longitudMensaje, 0);
	mensaje[longitudMensaje] = '\0';

	return mensaje;
}

void logearFinalizacionCpu(uint32_t socketCpu) {
	char* mensajeCpu = recibirMensaje(socketCpu);
	log_info(planificadorLog, "Mensaje de cpu: %s", mensajeCpu);
	free(mensajeCpu);
	sem_wait(&mutex);
	int _cpuBySocket(t_hilosConectados *hilosConectados) {
		if (hilosConectados->socketHilo == socketCpu)
			return 1;
		else
			return 0;
	}
	//Vuelvo a poner el cpu como disponible
	t_list *cpuPlanificado = list_filter(cpu_ocupados, (void*) _cpuBySocket);
	t_hilosConectados *cpu = list_get(cpu_ocupados, 0);
	cpu->estadoHilo = 0; // Ponemos hilo en estado disponible
	int _pcbByCpuPid(t_pcb *proc_ejecutado) {
		if (cpu->idProceso == proc_ejecutado->idProceso)
			return 1;
		else
			return 0;
	}
	t_list *pcbFinalizado = list_filter(proc_ejecutados, (void*) _pcbByCpuPid);
	t_pcb *pcb = list_get(pcbFinalizado, 0);
	pcb->estadoProceso = 2;
	list_remove_by_condition(cpu_ocupados, (void*) _cpuBySocket);
	list_add_all(cpu_listos, cpuPlanificado);
	sem_post(&sincrocpu); // Aumento semaforo cpu
	sem_post(&mutex);
}

char* serializarPCB(t_pcb *pcb, uint32_t *totalPaquete) {
	uint32_t tamanioenteros, tamaniopath, path;
	tamanioenteros = 4 * sizeof(uint32_t); //codigo+4 int de pcb
	tamaniopath = sizeof(uint32_t);
	path = strlen(pcb->path);
	*totalPaquete = tamanioenteros + tamaniopath + path;
	char *paqueteSerializado = malloc(*totalPaquete);
	int offset = 0;
	int medidaAMandar;

	medidaAMandar = sizeof(pcb->idProceso);
	memcpy(paqueteSerializado + offset, &(pcb->idProceso), medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = sizeof(pcb->estadoProceso);
	memcpy(paqueteSerializado + offset, &(pcb->estadoProceso), medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = sizeof(pcb->contadorPuntero);
	memcpy(paqueteSerializado + offset, &(pcb->contadorPuntero), medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = sizeof(pcb->cantidadInstrucciones);
	memcpy(paqueteSerializado + offset, &(pcb->cantidadInstrucciones),
			medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, &path, medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = path;
	memcpy(paqueteSerializado + offset, pcb->path, medidaAMandar);
	offset += medidaAMandar;

	return paqueteSerializado;
}

void replanificar(uint32_t socketCpu){
	//Recibo el Mensaje a logear
	char* mensajeALogear = recibirMensaje(socketCpu);
	log_info(planificadorLog, "Mensaje de cpu: %s", mensajeALogear);
	free(mensajeALogear);
	//Recibo el pcb bloqueado
	t_pcb* pcbBloqueado = recibirPcb(socketCpu);

	int _cpuById(t_hilosConectados *cpu_ocupado) {
		if (pcbBloqueado->idProceso == cpu_ocupado->idProceso)
			return 1;
		else
			return 0;
	}
	//Modifico lista proc y cpu
//	t_list *pcbBloqueadoLista = list_filter(proc_ejecutados, (void*) _pcbById);
	list_remove_and_destroy_element(proc_ejecutados, 0, (void*) pcbDestroy);
	pcbBloqueado->estadoProceso = 0;
//	t_list *cpuOcupado = list_filter(cpu_ocupados, (void*) _cpuById);
	t_hilosConectados *cpuOcupado = list_remove_by_condition(cpu_ocupados, (void*) _cpuById);
	cpuOcupado->estadoHilo = 0;
	cpuOcupado->path = "";
	list_add(cpu_listos, cpuOcupado);
	list_add(proc_listos, pcbBloqueado);
	sem_post(&sincrocpu); // Aumento semaforo cpu
	sem_post(&sincroproc); // Aumento semaforo cpu
}

void pcbDestroy(t_pcb *self) {
    free(self->path);
    free(self);
}

t_pcb* recibirPcb(uint32_t socketCpu){
	int32_t status = 0;
	t_pcb* pcbRecibido = malloc(sizeof(t_pcb));

	//Recibe mensaje de Planificador: PCB

	status = recv(socketCpu,&(pcbRecibido->idProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(planificadorLog,"Error al recibir PCB.","ERROR");
	status = recv(socketCpu,&(pcbRecibido->estadoProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(planificadorLog,"Error al recibir PCB.","ERROR");
	status = recv(socketCpu,&(pcbRecibido->contadorPuntero),sizeof(uint32_t),0);
	if (status <= 0) log_error(planificadorLog,"Error al recibir PCB.","ERROR");
	status = recv(socketCpu,&(pcbRecibido->cantidadInstrucciones),sizeof(uint32_t),0);
	if (status <= 0) log_error(planificadorLog,"Error al recibir PCB.","ERROR");
	status = recv(socketCpu,&(pcbRecibido->tamaniopath),sizeof(uint32_t),0);
	pcbRecibido->path = malloc(pcbRecibido->tamaniopath + 1);
	status = recv(socketCpu,pcbRecibido->path,pcbRecibido->tamaniopath,0);
	if (status <= 0) log_error(planificadorLog,"Error al recibir PCB.","ERROR");

	//memcpy(pcbRecibido->path[tamanioChar+1],'\0',1);
	pcbRecibido->path[pcbRecibido->tamaniopath] = '\0';
	//printf("path: %s",pcbRecibido->path);
	return pcbRecibido;
}

void creoCpu(uint32_t socketCpu){
	t_hilosConectados *hilosConectados = malloc(sizeof(t_hilosConectados));
	hilosConectados->socketHilo = socketCpu;
	hilosConectados->estadoHilo = 0; //0-disponible 1-Ejecutando
	hilosConectados->path = NULL;
	hilosConectados->idProceso = 0; // inicializo en 0
	sem_wait(&mutex);
	list_add(cpu_listos, hilosConectados);
	sem_post(&mutex);
	sem_post(&sincrocpu); // Aumento semaforo cpu
	log_info(planificadorLog,"Se conecto Cpu con socketId: %d", socketCpu);
}

//Agrego socket Padre y le informo el tipo de planificacion
void creoPadre(uint32_t socketProcesado){
	socketCpuPadre = socketProcesado;
	//Envio el tipo de planificacion al cpu
	uint32_t *totalPaquete = malloc(sizeof(uint32_t));
	char* tipoPlanificacion = serializarTipoPlanificaion(totalPaquete);
	char* mensaje = malloc(*totalPaquete);
	memcpy(mensaje, tipoPlanificacion, *totalPaquete); //NO Entiendo para que es esta linea(marian)
	int sendByte = send(socketCpuPadre, mensaje, *totalPaquete, 0);
	if (sendByte < 0) {
		log_error(planificadorLog, "Error al enviar el tipo de planificacion", "ERROR");
	}

}
char* serializarTipoPlanificaion(uint32_t *totalPaquete) {
	uint32_t codigo, tipoPlanificacion, quantum;
	codigo = 0;
	quantum = configuracion->quantum;
	if(!strcmp(configuracion->algoritmoPlanificacion,"FIFO")){ //si es fifo
		*totalPaquete = 2 * sizeof(uint32_t);
		tipoPlanificacion = 0; //Tipo FIFO
	}
	else {//si es RoundRobin
		*totalPaquete = 3 * sizeof(uint32_t);
		tipoPlanificacion = 1; //Tipo Round Robin
	}

	char *paqueteSerializado = malloc(*totalPaquete);
	int offset = 0;
	int medidaAMandar;

	medidaAMandar = sizeof(codigo);
	memcpy(paqueteSerializado + offset, &codigo, medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = sizeof(tipoPlanificacion);
	memcpy(paqueteSerializado + offset, &tipoPlanificacion, medidaAMandar);
	offset += medidaAMandar;

	if(tipoPlanificacion == 1){
		medidaAMandar = sizeof(quantum);
		memcpy(paqueteSerializado + offset, &quantum, medidaAMandar);
		offset += medidaAMandar;
	}

	return paqueteSerializado;
}

uint32_t crearSocketReceptor() {
	uint32_t socketReceptor;
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	char* puertoEscuchaString;
	puertoEscuchaString = string_itoa(configuracion->puertoEscucha);
	//Lleno la estructura de tipo addrinfo
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, puertoEscuchaString, &hints, &serverInfo);
	free(puertoEscuchaString);
	socketReceptor = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	int setOpcion = 1;
	if (setsockopt(socketReceptor, SOL_SOCKET, SO_REUSEADDR, &setOpcion,
			sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	bind(socketReceptor, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return socketReceptor;
}
