#include "cpu.h"

int conectarCPUPadreAPlanificador(){
	socketPlanificadorPadre = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);
	int32_t validationConnection = connect_to_server(socketPlanificadorPadre);
	if (validationConnection != 0)
	{
		printf("No se ha podido conectar correctamente al Planificador.\n");
		return EXIT_FAILURE;
	}
	enviarCodigoOperacion(socketPlanificadorPadre,CONEXION_CPU_PADRE);
	//Recibe respuesta
	printf("Esperando respuesta de Planificador\n");
	uint32_t codigo = deserializarEnteroSinSigno(socketPlanificadorPadre);
	if (codigo == CFG_INICIAL_PLN)
	{
		configCPUPadre.tipoPlanificacion = deserializarEnteroSinSigno(socketPlanificadorPadre);//0: FIFO, 1: RR
		if(configCPUPadre.tipoPlanificacion==1){
			configCPUPadre.quantum = deserializarEnteroSinSigno(socketPlanificadorPadre);
		}else{
			configCPUPadre.quantum = 0;
		}

		printf("Tipo de planificacion: %d , quantum: %d \n",configCPUPadre.tipoPlanificacion,configCPUPadre.quantum);
	}
	pthread_t hiloPorcentaje;
	int32_t hilo;
	hilo = pthread_create(&hiloPorcentaje, NULL,(void*) enviarPorcentaje, NULL);
	if (hilo) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", hilo);
		printf("Se cerrara el programa");
		return EXIT_FAILURE;
	}
	while(1){
//		uint32_t codigoRecibido = deserializarEnteroSinSigno(socketPlanificadorPadre);
		int32_t codigoRecibido;
		int32_t status = recv(socketPlanificadorPadre->fd, &codigoRecibido, sizeof(int32_t), 0);

		if(status == -1 || status == 0)
		{
			codigoRecibido = ANORMAL;
			printf("error, recibi mal el cod");
			break;
		}
		if (codigoRecibido == PORCENTAJES_CPU)
			sem_post(&semCpuPadre);
	}
	return EXIT_SUCCESS;
}

void* ConectarAPlanificador()
{
	sock_t* socketCliente = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);

	int32_t conexionPlanificador = connect_to_server(socketCliente);
	if (conexionPlanificador != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Planificador");
		return (void*)EXIT_FAILURE;
	}
	char message[1024];
	int status = 0;

	status = recv(socketCliente->fd, (void*)message, PAQUETE, 0);
	if(status > 0 ) printf("Mensaje de Planificador: %s \n",message);
	printf("Enviar mensaje a Administrador de memoria \n");


	sock_t* socketAAdminMemoria = create_client_socket(configuracion->ipMemoria,configuracion->puertoMemoria);
	int32_t conexionAdminMemoria = connect_to_server(socketAAdminMemoria);
	if (conexionAdminMemoria != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Planificador");
		return (void*)EXIT_FAILURE;
	}
	send(socketAAdminMemoria->fd, (void*)message, strlen(message) + 1, 0);
	sleep(10);
	clean_socket(socketCliente);
	clean_socket(socketAAdminMemoria);
	return NULL;
}


/** Funcion que crea un socket para recibir la peticion
 * del Planificador, para la ejecucion de un mProc.
 *
 * @return estructura deserializada que comparten Planificador y CPU
 */
t_pcb* escucharPlanificador(sock_t* socketPlanificador){
	int32_t status = 0;
	t_pcb* pcbRecibido = malloc(sizeof(t_pcb));

	//Recibe mensaje de Planificador: PCB
	uint32_t tamanioChar;
	status = recv(socketPlanificador->fd,&(pcbRecibido->idProceso),sizeof(uint32_t),0);
	if (status <= 0){
		log_error(CPULog,"Error al recibir PCB.","ERROR");
		return NULL;
	}
	status = recv(socketPlanificador->fd,&(pcbRecibido->estadoProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(pcbRecibido->contadorPuntero),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(pcbRecibido->cantidadInstrucciones),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(tamanioChar),sizeof(uint32_t),0);
	pcbRecibido->path = malloc(tamanioChar + 1);
	status = recv(socketPlanificador->fd,pcbRecibido->path,tamanioChar,0);
	pcbRecibido->path[tamanioChar] = '\0';
	if (status <= 0) {log_error(CPULog,"Error al recibir PCB.","ERROR"); free(pcbRecibido);}

	return pcbRecibido;
}

/** Función que crea un socket para enviar la notificacion al Admin
 * de Memoria, cuando se solicitó el comando "iniciar"
 * @param cantidad de páginas
 * @return int
 * 				-1 Fallo: falta de espacio
 * 				1 Fallo: otro motivo
 * 				0 Exitoso
 */
char* informarAdminMemoriaComandoIniciar(char* cantidadPaginas, int32_t pid,sock_t* socketMemoria){
	int32_t status;
	int32_t entero;
	//Envia aviso al Adm de Memoria: comando Iniciar.

	int32_t cabecera = INICIAR;
	int32_t paginas = atoi(cantidadPaginas);
	uint32_t offset=0;
	uint32_t tamanio = sizeof(cabecera) + sizeof(pid) + sizeof(paginas);
	char* message = malloc(tamanio);
	memcpy(message, &cabecera, sizeof(cabecera));
	offset = sizeof(cabecera);
	memcpy(message + offset, &pid, sizeof(pid));
	offset = offset + sizeof(pid);
	memcpy(message + offset, &paginas, sizeof(paginas));
	offset = offset + sizeof(paginas);
	status = send(socketMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje iniciar al Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje iniciar correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta
	status = recv(socketMemoria->fd,&entero,sizeof(int32_t),0);
	cabecera = RESPUESTA_PLANIFICADOR_LOGEAR;
	char* mensaje = string_new();
	if(entero==PEDIDO_ERROR){//mProc X - Fallo
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Fallo.\n");
		log_error(CPULog,"Error por falta de memoria. Se finalizará el proceso.","ERROR");
	}else{//mProc X - Iniciado
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Iniciado.\n");
		printf("inicio correctamente\n");
	}
	return mensaje;
}


/** Función que crea un socket para enviar la notificacion al Admin
 * de Memoria, cuando se solicitó el comando "finalizar"
 * @param id del proceso
 * @return int
 * 				0 Fallo
 * 				1 Exito
 */
char* informarAdminMemoriaComandoFinalizar(int32_t pid,char* resultadosDeEjecuciones, sock_t* socketPlanificador,sock_t* socketMemoria){
	int32_t status;
	int32_t entero;
	//Envia aviso al Adm de Memoria: comando Finalizar.
	int32_t cabecera = FINALIZAR;
	uint32_t offset=0;
	uint32_t tamanio = sizeof(cabecera) + sizeof(pid);
	char* message = malloc(tamanio);
	memcpy(message, &cabecera, sizeof(cabecera));
	offset = sizeof(cabecera);
	memcpy(message + offset, &pid, sizeof(pid));
	offset = offset + sizeof(pid);
	status = send(socketMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje finalizar al Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje finalizar correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta
	status = recv(socketMemoria->fd,&entero,sizeof(int32_t),0);
	//mProc X finalizado
	cabecera = RESPUESTA_PLANIFICADOR_FIN_EJECUCION;
	char* mensaje = string_new();
	offset = 0;
	string_append(&mensaje,resultadosDeEjecuciones);
	string_append(&mensaje, "mProc ");
	string_append(&mensaje, string_itoa(pid));
	string_append(&mensaje, " - Finalizado.\0");
	uint32_t longitudMensaje = strlen(mensaje);
	uint32_t tamanio2 = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
	char* message2 = malloc(tamanio2);
	memcpy(message2, &cabecera, sizeof(cabecera)); //Codigo
	offset = sizeof(cabecera);
	memcpy(message2 + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
	offset = offset + sizeof(uint32_t);
	memcpy(message2 + offset, mensaje, longitudMensaje); //Mensaje
	offset = offset + longitudMensaje;
	status = send(socketPlanificador->fd,message2,tamanio2,0);
	free(message2);
	return "FIN";
}

char* informarAdminMemoriaComandoLeer(int32_t pid, char* pagina, sock_t* socketMemoria){
	int32_t status;
	int32_t numeroPagina = atoi(pagina);
	int32_t cabecera = LEER;
	uint32_t offset=0;
	uint32_t tamanio = sizeof(cabecera) + sizeof(pid) + sizeof(numeroPagina);
	char* message = malloc(tamanio);
	memcpy(message, &cabecera, sizeof(cabecera));
	offset = sizeof(cabecera);
	memcpy(message + offset, &pid, sizeof(pid));
	offset = offset + sizeof(pid);
	memcpy(message + offset, &numeroPagina, sizeof(numeroPagina));
	offset = offset + sizeof(numeroPagina);
	status = send(socketMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje leer Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje leer correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta :(exitoso=1, longitud y contenido)
	uint32_t lecturaExitosa = deserializarEnteroSinSigno(socketMemoria);
	if(lecturaExitosa==PEDIDO_ERROR){
		return PEDIDO_ERROR;
		log_error(CPULog,"El pedido de lectura no fue exitoso.","ERROR");
	}

	char* contenido = recibirMensaje(socketMemoria);
	log_info(CPULog,"[PID:%s] Lectura realizada. Contenido: %s",string_itoa(pid),contenido);

	//mProc 10 - Pagina 2 leida: contenido
	cabecera = RESPUESTA_PLANIFICADOR_LOGEAR;
	char* mensaje = string_new();
	offset = 0;
	string_append(&mensaje, "mProc ");
	string_append(&mensaje, string_itoa(pid));
	string_append(&mensaje, " - Pagina ");
	string_append(&mensaje, pagina);
	string_append(&mensaje," leida: ");
	string_append(&mensaje,contenido);
	string_append(&mensaje,"\n");
	return mensaje;
}

char* informarAdminMemoriaComandoEscribir(int32_t pid, int32_t numeroPagina,char* texto,sock_t* socketMemoria){
	int32_t status;
	int32_t entero;
	//Envia aviso al Adm de Memoria: comando Escribir.

	int32_t cabecera = ESCRIBIR;

	char*textoAEscribir=texto;
	uint32_t longitudMensaje= strlen (textoAEscribir);

	uint32_t offset=0;
	uint32_t tamanio = sizeof(cabecera) + sizeof(pid) + sizeof(numeroPagina) + longitudMensaje +  sizeof(uint32_t);
	char* message = malloc(tamanio);
	memcpy(message, &cabecera, sizeof(cabecera));
	offset = sizeof(cabecera);
	memcpy(message + offset, &pid, sizeof(pid));
	offset = offset + sizeof(pid);
	memcpy(message + offset, &numeroPagina, sizeof(numeroPagina));
	offset = offset + sizeof(numeroPagina);
	memcpy(message + offset,&longitudMensaje, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(message + offset, textoAEscribir, longitudMensaje);
	offset = offset + sizeof(longitudMensaje);
	status = send(socketMemoria->fd,message,tamanio,0);

	free(message);

	if(!status)	{
		printf("No se envió el mensaje escribir al Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje escribir correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta
	status = recv(socketMemoria->fd,&entero,sizeof(int32_t),0);
	cabecera = RESPUESTA_PLANIFICADOR_LOGEAR;
	char* mensaje = string_new();
	offset = 0;
	if(entero==PEDIDO_ERROR){//mProc X - Fallo Escritura
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Fallo Escritura.\n");
		log_error(CPULog,"Error en la escritura del proceso.","ERROR");
	} else {
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Página ");
		string_append(&mensaje, string_itoa(numeroPagina));
		string_append(&mensaje, " escrita: ");
		string_append(&mensaje, textoAEscribir);
		string_append(&mensaje, "\n");
		log_info(CPULog,"Se escribió %s en el Proceso %s, Página %d",textoAEscribir, string_itoa(pid), numeroPagina);
	}
	return mensaje;
}

void enviarCodigoOperacion(sock_t* socket, int32_t entero){
	int32_t enviado = send(socket->fd, &entero, sizeof(int32_t), 0);
	if(enviado!=sizeof(int32_t)){
		printf("No se envió correctamente la información entera\n");
		log_error(CPULog,"Error al enviar codigo de operacion.","ERROR");
		return;
	}
}

uint32_t deserializarEnteroSinSigno(sock_t* socket)
{
	uint32_t enteroSinSigno;
	int32_t status = recv(socket->fd, &enteroSinSigno, sizeof(uint32_t), 0);
	if(status == -1 || status == 0)
	{
		enteroSinSigno = ANORMAL;
		printf("Se recibio mal el entero.");
	}
	return enteroSinSigno;
}

char* recibirMensaje(sock_t* socket){
	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje;
	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	recv(socket->fd, &longitudMensaje, sizeof(int32_t), 0);
	char* mensaje = malloc(longitudMensaje+1);
	recv(socket->fd, mensaje, longitudMensaje, 0);
	mensaje[longitudMensaje+1]='\0';
	return mensaje;
}

char* serializarFinQuantum(t_pcb *pcb, uint32_t *totalPaquete, char* resultadosDeEjecuciones) {
	uint32_t cabecera = FIN_QUANTUM;
	uint32_t offset=0;
	uint32_t tamanioenterosPCB, tamaniopath, tamanioCabecera, path, tamanioMensajes, tamanioCabeceraMensaje;
	tamanioenterosPCB = 4 * sizeof(uint32_t); //4 int de pcb
	tamaniopath = sizeof(uint32_t);
	tamanioCabecera = sizeof(cabecera);
	path = strlen(pcb->path);
	tamanioCabeceraMensaje = sizeof(uint32_t);
	tamanioMensajes = strlen(resultadosDeEjecuciones);
	*totalPaquete = tamanioCabecera + tamanioCabeceraMensaje + tamanioMensajes + tamanioenterosPCB + tamaniopath + path;
	char *paqueteSerializado = malloc(*totalPaquete);

	int medidaAMandar;
	medidaAMandar = tamanioCabecera;
	memcpy(paqueteSerializado + offset, &cabecera, medidaAMandar);
	offset += medidaAMandar;
	medidaAMandar = tamanioCabeceraMensaje ;
	memcpy(paqueteSerializado + offset, &tamanioMensajes, medidaAMandar);
	offset += medidaAMandar;
	medidaAMandar = tamanioMensajes;
	memcpy(paqueteSerializado + offset, resultadosDeEjecuciones, medidaAMandar);
	offset += medidaAMandar;
	return serializarPCB(pcb, offset, paqueteSerializado);
}

char* serializarPCB(t_pcb *pcb,uint32_t offset,char *paqueteSerializado) {
	uint32_t path = strlen(pcb->path);
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
	memcpy(paqueteSerializado + offset, &(pcb->cantidadInstrucciones),medidaAMandar);
	offset += medidaAMandar;
	medidaAMandar = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, &path, medidaAMandar);
	offset += medidaAMandar;
	medidaAMandar = path;
	memcpy(paqueteSerializado + offset, pcb->path, medidaAMandar);
	offset += medidaAMandar;
	return paqueteSerializado;
}

int informarPlanificadorLiberacionCPU(t_pcb* pcb,char* resultadosDeEjecuciones,sock_t* socketPlanificador){
	uint32_t *totalPaquete = malloc(sizeof(uint32_t));
	char* pcbSerializado = serializarFinQuantum(pcb, totalPaquete, resultadosDeEjecuciones);
	char* mensaje = malloc(*totalPaquete);
	memcpy(mensaje, pcbSerializado, *totalPaquete);
	int sendByte = send(socketPlanificador->fd, mensaje, *totalPaquete, 0);
	if (sendByte < 0) {
		log_error(CPULog, "Error al enviar el proc/pcb al Planificador","ERROR");
	}
	free(mensaje);
	free(totalPaquete);
	return EXIT_SUCCESS;
}

char* informarEntradaSalida(t_pcb* pcb, int32_t tiempo, char* resultadosDeEjecuciones,sock_t* socketPlanificador){
	int32_t status;
	int32_t cabecera = ENTRADA_SALIDA;
	uint32_t offset=0;
	int medidaAMandar;
	char* mensaje = string_new();
	string_append(&mensaje,resultadosDeEjecuciones);
	string_append(&mensaje, "mProc ");
	string_append(&mensaje, string_itoa(pcb->idProceso));
	string_append(&mensaje, " en entrada-salida de tiempo ");
	string_append(&mensaje, string_itoa(tiempo));
	string_append(&mensaje, "\n");

	uint32_t longitudMensaje = strlen(mensaje);
	uint32_t tamanioenteros, tamaniopath, path;
	tamanioenteros = 4 * sizeof(uint32_t); //codigo+4 int de pcb
	tamaniopath = sizeof(uint32_t);
	path = strlen(pcb->path);
	uint32_t tamanio = sizeof(cabecera) + sizeof(tiempo) + sizeof(uint32_t) + longitudMensaje + tamanioenteros + tamaniopath + path;

	char* paqueteSerializado = malloc(tamanio);
	/* Envio Cabecera-Retardo-MensajeALog */
	medidaAMandar = sizeof(int32_t);
	memcpy(paqueteSerializado, &cabecera,medidaAMandar);
	offset += medidaAMandar;
	medidaAMandar = sizeof(tiempo);
	memcpy(paqueteSerializado + offset, &tiempo, medidaAMandar);
	offset += medidaAMandar;
	medidaAMandar = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, &longitudMensaje, medidaAMandar); //longitud mensaje
	offset += medidaAMandar;
	medidaAMandar = longitudMensaje;
	memcpy(paqueteSerializado + offset, mensaje, medidaAMandar);
	offset += medidaAMandar;
	/* Envio PCB */
	medidaAMandar = sizeof(pcb->idProceso);
	memcpy(paqueteSerializado + offset, &(pcb->idProceso),medidaAMandar);
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
	char* mensajeEnviar = malloc(tamanio);
	memcpy(mensajeEnviar, paqueteSerializado, tamanio);
	status = send(socketPlanificador->fd,mensajeEnviar,tamanio,0);
	free(paqueteSerializado);
	return "FIN";
}

void enviarPorcentaje(){
	while (1) {
		sem_wait(&semCpuPadre);
//		pthread_mutex_lock(&mutexListaCpus);
		uint32_t cabecera = PORCENTAJES_CPU;
		uint32_t offset = 0;
		int32_t status;

		//char listaTemporal[TAMINSTRUCCION]="HARDCODEADO";

//VA TOMANDO DE LISTA CPU LOS DIFERENTES CPU (id, % y tiempo), CONCATENA Y ARMA EL STRING.//AUMENTA EL INDICE Y LO COMPLETA CON LOS DATOS RESTANTES DE CPU.
		char* listaTemporal = malloc(1000);
		t_CPUsConectados* temporal;
		uint32_t indice= 0;

		while (indice < configuracion->cantidadHilos) {
			temporal = list_get(listaCPU, indice);
			strcat(listaTemporal, "CPU:");
			strcat(listaTemporal, string_itoa(temporal->numeroCPU));
			strcat(listaTemporal, ", Porcentaje de uso:");
			strcat(listaTemporal, string_itoa(temporal->porcentajeProcesado));
			strcat(listaTemporal, "\n");
			indice++;
		}


//		char* listaTemporal= string_new();
//		t_CPUsConectados* temporal;
//		uint32_t indice= 0;
//
//		while (indice < configuracion->cantidadHilos) {
//			temporal = list_get(listaCPU, indice);
//			string_append(&listaTemporal, "CPU:");
//			string_append(&listaTemporal, string_itoa(temporal->idCPU));
//			string_append(&listaTemporal, ", Porcentaje de uso:");
//			string_append(&listaTemporal, string_itoa(temporal->porcentajeProcesado));
//			string_append(&listaTemporal, "\n");
//			indice++;
//		}

		int32_t tamListaTemp = strlen(listaTemporal);
		int32_t tamanio = sizeof(cabecera) + sizeof(int32_t) + tamListaTemp;
		char* message = malloc(tamanio);
		memcpy(message, &cabecera, sizeof(cabecera));
		offset = sizeof(cabecera);
//		printf("tam lista %d", tamListaTemp);
//		printf("%s",listaTemporal);
		memcpy(message + offset, &tamListaTemp, sizeof(int32_t));
		offset = offset + sizeof(int32_t);
		memcpy(message + offset, listaTemporal, tamListaTemp);
		offset = offset + sizeof(tamListaTemp);
		status = send(socketPlanificadorPadre->fd,message,tamanio,0);
//		printf("socket plani padre %d \n",socketPlanificadorPadre->fd);
//		printf("status: %d \n", status);
		if (status < 0) {
			printf("Error al enviar el porcentaje a Planificador");
		}
		free(message);
		free(listaTemporal);
//		pthread_mutex_unlock(&mutexListaCpus);
	}
}
