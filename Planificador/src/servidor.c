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
	uint32_t *hiloPlanificador = malloc(sizeof(uint32_t));
	*hiloPlanificador = 0;
	uint32_t *hiloBloqueado = malloc(sizeof(uint32_t));
	*hiloBloqueado = 0;
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
						generoHiloPlanificador(hiloPlanificador);
						break;
					case LOGEARRESULTADOCPU:
						logearResultadoCpu(socketProcesado);
						break;
					case FINQUANTUM:
						replanificar(socketProcesado);
						break;
					case ENTRADASALIDA:
						bloquearProceso(socketProcesado, hiloBloqueado);
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

/***** Funcion consume PCBs mientras halla CPUs Disponibles *****/
void consumirRecursos() {
	while (1) {
		sem_wait(&sincrocpu);
		sem_wait(&sincroproc);
		while (list_size(cpu_listos) > 0 && list_size(proc_listos) > 0) {
			sem_wait(&mutex);
			//Agarro el primero de cada la cola
			t_list *cpu_ocupado = list_take_and_remove(cpu_listos, 1);
			t_list *proc_ejecutado = list_take_and_remove(proc_listos, 1);
			t_pcb *pcb = list_get(proc_ejecutado, 0);
			t_hilosConectados *cpu = list_get(cpu_ocupado, 0);
			pcb->estadoProceso = 1; //PCB en estado de ejecucion
			cpu->estadoHilo = 1; // CPU en estado de ejecucion
			cpu->idProceso = pcb->idProceso; //Asigno al cpu su IdProceso que va a ejec.
			//Mandar por socket al cpu el proc para ejecutar
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

/***** Funcion replanifica PCB cuando CPU vuelve por Quantum *****/
void replanificar(uint32_t socketCpu) {
	sem_wait(&mutex);
	//Recibo el Mensaje a logear de ejecucion por Quantum
	char* mensajeALogear = recibirMensaje(socketCpu);
	log_info(planificadorLog, "Mensaje de cpu: %s", mensajeALogear);
	free(mensajeALogear);
	//Recibo el pcb a replanificar
	t_pcb* pcbReplanificar = recibirPcb(socketCpu);
	//Metodos para buscar
	int _pcbById(t_pcb *proc_ejecutado) {
		if (pcbReplanificar->idProceso == proc_ejecutado->idProceso)
			return 1;
		else
			return 0;
	}
	int _cpuById(t_hilosConectados *cpu_ocupado) {
		if (pcbReplanificar->idProceso == cpu_ocupado->idProceso)
			return 1;
		else
			return 0;
	}
	//Reviso el Flag de finalizacion

	t_pcb *pcbFinListo = list_find(proc_ejecutados, (void*) _pcbById);
	if(pcbFinListo != NULL && pcbFinListo->flagFin == 1)
		pcbReplanificar->contadorPuntero = pcbReplanificar->cantidadInstrucciones;
	//Libero CPU y PCB y vuelven a encolarse
	list_remove_and_destroy_by_condition(proc_ejecutados, (void*) _pcbById, (void*) pcbDestroy);
	pcbReplanificar->estadoProceso = 0;
	t_hilosConectados *cpuOcupado = list_remove_by_condition(cpu_ocupados, (void*) _cpuById);
	cpuOcupado->estadoHilo = 0; //Asigno estado disponible
	cpuOcupado->path = "";
	list_add(cpu_listos, cpuOcupado);
	list_add(proc_listos, pcbReplanificar);
	sem_post(&mutex);
	sem_post(&sincrocpu); // Aumento semaforo cpu
	sem_post(&sincroproc); // Aumento semaforo Proc
}

void bloquearProceso(uint32_t socketCpu, uint32_t *hiloBloqueado){
	sem_wait(&mutex);
	//recibo retardo y mensaje
	int32_t retardo = deserializarEnteroSinSigno(socketCpu);
	//Recibo el Mensaje a logear
	char* mensajeALogear = recibirMensaje(socketCpu);
	log_info(planificadorLog, "Mensaje de cpu: %s", mensajeALogear);
	free(mensajeALogear);
	//recibo pcb
	t_pcb* pcbBloqueado = recibirPcb(socketCpu);

	//Metodos para buscar
	int _pcbById(t_pcb *proc_ejecutado) {
		if (pcbBloqueado->idProceso == proc_ejecutado->idProceso)
			return 1;
		else
			return 0;
	}
	int _cpuById(t_hilosConectados *cpu_ocupado) {
		if (pcbBloqueado->idProceso == cpu_ocupado->idProceso)
			return 1;
		else
			return 0;
	}
	//Libero CPU ocupada
	t_hilosConectados *cpuOcupado = list_remove_by_condition(cpu_ocupados, (void*) _cpuById);
	//Antes me fijo si tenia Flag de finalizar
	t_pcb *pcbFinListo = list_find(proc_ejecutados, (void*) _pcbById);
	if(pcbFinListo != NULL && pcbFinListo->flagFin == 1)
		pcbBloqueado->flagFin = 1;
	//Sacar pcb de ejecutados - Meterlo en bloqueados
	list_remove_and_destroy_by_condition(proc_ejecutados, (void*) _pcbById, (void*) pcbDestroy);
	pcbBloqueado->estadoProceso = 3; //Asigno estado Bloqueado
	pcbBloqueado->retardo = retardo;
	list_add(proc_bloqueados, pcbBloqueado);
	//Asigno CPU nuevamente libre
	list_add(cpu_listos, cpuOcupado);
	sem_post(&mutex);
	sem_post(&sincrocpu); //Aumento semaforo cpu
	sem_post(&sincroBloqueados);
	//UN solo hilo que se cree con la primer E/S y luego siga esperando pcbs bloqueados
	pthread_t hiloBloqueados;
	int respPlanificador = 0;
	if (!*hiloBloqueado) {
		*hiloBloqueado = 1;
		respPlanificador = pthread_create(&hiloBloqueados, NULL, (void *) iniciarHiloBloqueados, NULL);
	}
	if (respPlanificador) {
		fprintf(stderr, "Error- Iniciar Hilo Bloqueados %d\n", respPlanificador);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}
}

/* Hilo de PCB Bloqueados, hace sleep y vuelve PCB a ready */
void iniciarHiloBloqueados() {
	while (1) {
		sem_wait(&sincroBloqueados);
		while (list_size(proc_bloqueados) > 0) {
			//Agarro el primero de cada la cola
			t_pcb *pcbBloqueado = list_get(proc_bloqueados, 0);
			//hago sleep
			sleep(pcbBloqueado->retardo);
			//Reviso el Flag de finalizacion
			sem_wait(&mutex);
			if(pcbBloqueado != NULL && pcbBloqueado->flagFin == 1)
				pcbBloqueado->contadorPuntero = pcbBloqueado->cantidadInstrucciones;

			list_remove(proc_bloqueados, 0);
			//Lo vuelvo a meter en cola de listos
			pcbBloqueado->estadoProceso = 0; //PCB en estado de espera
			list_add(proc_listos, pcbBloqueado);
			sem_post(&mutex);
			sem_post(&sincroproc); //Aumento semaforo Proc
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
	sem_wait(&mutex);
	char* mensajeCpu = recibirMensaje(socketCpu);
	log_info(planificadorLog, "Mensaje de cpu: %s", mensajeCpu);
	free(mensajeCpu);
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
	t_pcb *pcb = list_find(proc_ejecutados, (void*) _pcbByCpuPid);
	pcb->estadoProceso = 2;
	list_remove_by_condition(cpu_ocupados, (void*) _cpuBySocket);
	list_add_all(cpu_listos, cpuPlanificado);
	sem_post(&mutex);
	sem_post(&sincrocpu); // Aumento semaforo cpu
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
	string_trim(&(configuracion->algoritmoPlanificacion));

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
