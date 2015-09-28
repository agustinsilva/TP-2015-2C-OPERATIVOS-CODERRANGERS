#include "planificador.h"

//falta optimizar con shared library sockets
void* iniciarServidor() {
	t_list *cpu_listos;
	t_list *cpu_ocupados;
	cpu_listos = list_create();
	cpu_ocupados = list_create();
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
	uint32_t cabecera ;
	char paquete[PAQUETE];
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
						printf("Se recibio nueva conexion Cpu %d \n",nuevoFd);

					}
				} else //Aca se ejecuta el socket procesado
				{
//					status = recv(socketProcesado, (void*) paquete, PAQUETE, 0);
//					if (status > 0) {
//						printf("%s", paquete);
//					}
//					if(strcmp(paquete,"1") == 0){
//						//Agrego cpu que se conecto y la mando a ejecutar
//						creoCpu(socketProcesado, cpu_listos);
//						generoHiloPlanificador(cpu_listos);
//					}
					cabecera = deserializarEnteroSinSigno(socketProcesado);
					switch (cabecera)
							{
								case AGREGARHILOCPU:
									creoCpu(socketProcesado, cpu_listos);
									generoHiloPlanificador(cpu_listos);
									break;
//								case ERROR:
//								printf("Finalizacion anormal de Planificador\n");
//								exit(1);
//								break;
//								default:
//									// Por si acaso
//									break;
							}
					if (cabecera <= 0) {
						printf("Se desconecto socket cpu %d \n",
								socketProcesado);
						close(socketProcesado);
						FD_CLR(socketProcesado, &set_maestro);
					}
				}
			}
		}
	}
	return NULL;
}

uint32_t deserializarEnteroSinSigno(uint32_t socket)
{
	uint32_t enteroSinSigno;
	uint32_t status = recv(socket, &enteroSinSigno, sizeof(uint32_t), 0);
	if(status == -1 || status == 0)
	{
		enteroSinSigno = status;
	}
	return enteroSinSigno;

}


void generoHiloPlanificador(t_list *cpu_listos){
	pthread_t hiloPlanificador;
	int respPlanificador = 0;

	respPlanificador = pthread_create(&hiloPlanificador,NULL,consumirRecursos, cpu_listos);

	if(respPlanificador)
	{
		fprintf(stderr,"Error- Iniciar servidor codigo de retorno %d\n",respPlanificador);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}
}

void consumirRecursos(t_list *cpu_listos){
	//mientras halla cpu listos, mandar a ejecutar pcb

	while (list_size(cpu_listos)>0 && list_size(proc_listos)>0){
		t_list *cpu_ocupado = list_take_and_remove(cpu_listos, 1);
		t_list *proc_ejecutado = list_take_and_remove(proc_listos, 1);
		//Mandar por socket al cpu el proc para ejecutar
		t_pcb *pcb = list_get(proc_ejecutado, 0);
		t_hilosConectados *cpu = list_get(cpu_ocupado, 0);
		uint32_t *totalPaquete = malloc(sizeof(uint32_t));

		char* pcbSerializado = serializarPCB(pcb,totalPaquete);

		char* mensaje = malloc(*totalPaquete);
		memcpy(mensaje,pcbSerializado,*totalPaquete);
		int sendhola = send(cpu->socketHilo, mensaje, *totalPaquete, 0);
		printf("%d",sendhola);
		free(mensaje);
		free(pcbSerializado);
		free(totalPaquete);

		//Asignar los cpu y proc usados en una lista de ocupados y ejecutados.
	}
}

char* serializarPCB(t_pcb *pcb, uint32_t *totalPaquete){
	uint32_t tamanioenteros,tamaniopath,path;
	tamanioenteros = 4 * sizeof(uint32_t); //codigo+4 int de pcb
	tamaniopath = sizeof(uint32_t);
	path = strlen(pcb->path);
	*totalPaquete = tamanioenteros+tamaniopath+path;
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
	memcpy(paqueteSerializado + offset, &(pcb->cantidadInstrucciones), medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, &path, medidaAMandar);
	offset += medidaAMandar;

	medidaAMandar = path;
	memcpy(paqueteSerializado + offset, pcb->path, medidaAMandar);
	offset += medidaAMandar;

	return paqueteSerializado;
}

void creoCpu(uint32_t socketCpu, t_list *cpu_listos){
	t_hilosConectados *hilosConectados = malloc(sizeof(t_hilosConectados));
	hilosConectados->socketHilo = socketCpu;
	hilosConectados->estadoHilo = 0; //0-disponible 1-Ejecutando
	hilosConectados->path = NULL;
	list_add(cpu_listos,hilosConectados);
	printf("Se creo hilo disponible para asignar\n");
}

uint32_t crearSocketReceptor()
{
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
	if (setsockopt(socketReceptor, SOL_SOCKET, SO_REUSEADDR, &setOpcion, sizeof(int))
				== -1)
		{
			perror("setsockopt");
			exit(1);
		}
	bind(socketReceptor, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar
	return socketReceptor;
}
