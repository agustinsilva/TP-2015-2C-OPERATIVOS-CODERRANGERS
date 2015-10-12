#include "cpu.h"

int conectarCPUPadreAPlanificador(){
	socketPlanificadorPadre = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);
	int32_t validationConnection = connect_to_server(socketPlanificadorPadre);
	if (validationConnection != 0 )
	{
		printf("No se ha podido conectar correctamente al Planificador.\n");
		return EXIT_FAILURE;
	}
	enviarCodigoOperacion(socketPlanificadorPadre,CONEXION_CPU_PADRE);
	//Recibe respuesta
	printf("esperando rta de planificador\n");
	uint32_t codigo = deserializarEnteroSinSigno(socketPlanificadorPadre);
	configCPUPadre.tipoPlanificacion = deserializarEnteroSinSigno(socketPlanificadorPadre);//0: FIFO, 1: RR
	if(configCPUPadre.tipoPlanificacion==1){
		configCPUPadre.quantum = deserializarEnteroSinSigno(socketPlanificadorPadre);
	}else{
		configCPUPadre.quantum = 0;
	}
	printf("tipo de planificacion: %d , quantum: %d \n",configCPUPadre.tipoPlanificacion,configCPUPadre.quantum);
	return EXIT_SUCCESS;
}

int conectarAAdministradorMemoria(){
	sock_t* clientSocketAdmin = create_client_socket(configuracion->ipMemoria,configuracion->puertoMemoria);
	int32_t conexionAdminMemoria = connect_to_server(clientSocketAdmin);
	if (conexionAdminMemoria != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Administrador de Memoria. ","ERROR");
		return EXIT_FAILURE;
	}
	socketAdminMemoria = clientSocketAdmin;
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
t_pcb* escucharPlanificador(){
	int32_t status = 0;
	t_pcb* pcbRecibido = malloc(sizeof(t_pcb));

	//Recibe mensaje de Planificador: PCB
	uint32_t tamanioChar;
	status = recv(socketPlanificador->fd,&(pcbRecibido->idProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(pcbRecibido->estadoProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(pcbRecibido->contadorPuntero),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(pcbRecibido->cantidadInstrucciones),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketPlanificador->fd,&(tamanioChar),sizeof(uint32_t),0);
	pcbRecibido->path = malloc(tamanioChar + 1);
	status = recv(socketPlanificador->fd,pcbRecibido->path,tamanioChar,0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
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
char* informarAdminMemoriaComandoIniciar(char* cantidadPaginas, int32_t pid){
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
	status = send(socketAdminMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje iniciar al Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje iniciar correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta
	status = recv(socketAdminMemoria->fd,&entero,sizeof(int32_t),0);
	cabecera = RESPUESTA_PLANIFICADOR_LOGEAR;
	char* mensaje = string_new();
	//offset = 0;
	if(entero==PEDIDO_ERROR){//mProc X - Fallo
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Fallo.\n");
		log_error(CPULog,"Error por falta de memoria. Se finalizará el proceso.","ERROR");
		/*uint32_t longitudMensaje = strlen(mensaje);
		uint32_t tamanio = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
		char* message2 = malloc(tamanio);
		memcpy(message2, &cabecera, sizeof(cabecera));
		offset = sizeof(cabecera);
		memcpy(message2 + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
		offset = offset + sizeof(uint32_t);
		memcpy(message2 + offset, mensaje, longitudMensaje);
		status = send(socketPlanificador->fd,message2,tamanio,0);
		free(message2);*/
	}else{//mProc X - Iniciado
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Iniciado.\n");
		/*uint32_t longitudMensaje = strlen(mensaje);
		uint32_t tamanio = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
		char* message3 = malloc(tamanio);
		memcpy(message3, &cabecera, sizeof(cabecera)); //Codigo
		offset = sizeof(cabecera);
		memcpy(message3 + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
		offset = offset + sizeof(uint32_t);
		memcpy(message3 + offset, mensaje, longitudMensaje); //Mensaje
		status = send(socketPlanificador->fd,message3,tamanio,0);
		free(message3);*/
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
char* informarAdminMemoriaComandoFinalizar(int32_t pid,char* resultadosDeEjecuciones){
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
	status = send(socketAdminMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje finalizar al Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje finalizar correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta
	status = recv(socketAdminMemoria->fd,&entero,sizeof(int32_t),0);

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

char* informarAdminMemoriaComandoLeer(int32_t pid, char* pagina){
	int32_t status;
	int32_t numeroPagina = atoi(pagina);
	//Envia aviso al Adm de Memoria: comando Finalizar.
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
	status = send(socketAdminMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje leer Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje leer correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta :(exitoso=1, longitud y contenido)
	uint32_t lecturaExitosa = deserializarEnteroSinSigno(socketAdminMemoria);
	if(lecturaExitosa==PEDIDO_ERROR){
		return PEDIDO_ERROR;
		log_error(CPULog,"El pedido de lectura no fue exitoso.","ERROR");
	}
	char* contenido = recibirMensaje(socketAdminMemoria);
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
	/*uint32_t longitudMensaje = strlen(mensaje);
	tamanio = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
	message = malloc(tamanio);
	memcpy(message, &cabecera, sizeof(cabecera)); //Codigo
	offset = sizeof(cabecera);
	memcpy(message + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
	offset = offset + sizeof(uint32_t);
	memcpy(message + offset, mensaje, longitudMensaje); //Mensaje
	offset = offset + longitudMensaje;
	status = send(socketPlanificador->fd,message,tamanio,0);
	free(message);
	free(contenido);*/
	return mensaje;
}

char* informarAdminMemoriaComandoEscribir(int32_t pid, int32_t numeroPagina,char* texto){
	int32_t status;
	int32_t entero;
	//Envia aviso al Adm de Memoria: comando Escribir.

	int32_t cabecera = ESCRIBIR; //DECIR A ELIANA QUE LE MANDAMOS LA CABECERA ESCRIBIR CUANDO ESCRIBE
	char*textoAEscribir=texto;

	uint32_t offset=0;
	uint32_t tamanio = sizeof(cabecera) + sizeof(pid) + sizeof(numeroPagina) + sizeof(textoAEscribir);
	char* message = malloc(tamanio);
	memcpy(message, &cabecera, sizeof(cabecera));
	offset = sizeof(cabecera);
	memcpy(message + offset, &pid, sizeof(pid));
	offset = offset + sizeof(pid);
	memcpy(message + offset, &numeroPagina, sizeof(numeroPagina));
	offset = offset + sizeof(numeroPagina);
	memcpy(message + offset, &textoAEscribir, sizeof(numeroPagina));
	offset = offset + sizeof(textoAEscribir);
	status = send(socketAdminMemoria->fd,message,tamanio,0);
	free(message);

	if(!status)	{
		printf("No se envió el mensaje escribir al Administrador de Memoria.\n");
	}
	else {
		printf("Se envió el mensaje escribir correctamente al Admin de Memoria.\n");
	}

	//Recibe respuesta
	status = recv(socketAdminMemoria->fd,&entero,sizeof(int32_t),0);
	cabecera = RESPUESTA_PLANIFICADOR_LOGEAR;
	char* mensaje = string_new();
	offset = 0;
	if(entero==PEDIDO_ERROR){//mProc X - Fallo Escritura
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Fallo Escritura.\n");
		log_error(CPULog,"Error en la escritura del proceso.","ERROR");

		/*uint32_t longitudMensaje = strlen(mensaje);
		uint32_t tamanio = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
		char* message2 = malloc(tamanio);
		memcpy(message2, &cabecera, sizeof(cabecera));
		offset = sizeof(cabecera);
		memcpy(message2 + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
		offset = offset + sizeof(uint32_t);
		memcpy(message2 + offset, mensaje, longitudMensaje);

		//Manda mensaje a Planificador del error de escritura
		status = send(socketPlanificador->fd,message2,tamanio,0);
		free(message2);*/
	} else {
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Página ");
		string_append(&mensaje, string_itoa(numeroPagina));
		string_append(&mensaje, " escrita: ");
		string_append(&mensaje, textoAEscribir);
		string_append(&mensaje, "\n");
		log_error(CPULog,"Se escribió %s en el Proceso %d, Página %d",textoAEscribir, pid, numeroPagina,"ERROR");

		/*uint32_t longitudMensaje = strlen(mensaje);
		uint32_t tamanio = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
		char* message2 = malloc(tamanio);
		memcpy(message2, &cabecera, sizeof(cabecera));
		offset = sizeof(cabecera);
		memcpy(message2 + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
		offset = offset + sizeof(uint32_t);
		memcpy(message2 + offset, mensaje, longitudMensaje);

		//Manda mensaje a Planificador informando que se escribió en la página.
		status = send(socketPlanificador->fd,message2,tamanio,0);
		free(message2);*/
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
	uint32_t status = recv(socket->fd, &enteroSinSigno, sizeof(uint32_t), 0);
	if(status == -1 || status == 0)
	{
		enteroSinSigno = ANORMAL;
	}
	return enteroSinSigno;
}

char* recibirMensaje(sock_t* socket){
	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje;
	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	recv(socket->fd, &longitudMensaje, sizeof(int32_t), 0);
	char* mensaje = (char*) malloc(longitudMensaje+1);
	recv(socket->fd, mensaje, longitudMensaje, 0);
	mensaje[longitudMensaje]='\0';
	return mensaje;
}

char* serializarPCB(t_pcb *pcb, uint32_t *totalPaquete, char* resultadosDeEjecuciones) {
	int32_t cabecera = FIN_QUANTUM;
	int32_t offset=0;
	uint32_t tamanioenteros, tamaniopath, tamanioCabecera, path, tamanioMensajes, mensajes;
	tamanioenteros = 4 * sizeof(uint32_t); //codigo+4 int de pcb
	tamaniopath = sizeof(uint32_t);
	tamanioCabecera = sizeof(cabecera);
	path = strlen(pcb->path);
	tamanioMensajes = sizeof(uint32_t);
	mensajes = strlen(resultadosDeEjecuciones);
	*totalPaquete = tamanioenteros + tamaniopath + tamanioCabecera + path + tamanioMensajes + mensajes;
	char *paqueteSerializado = malloc(*totalPaquete);

	int medidaAMandar;
	medidaAMandar = tamanioCabecera;
	memcpy(paqueteSerializado + offset, &cabecera, medidaAMandar);
	offset = medidaAMandar;
	medidaAMandar = tamanioMensajes;
	memcpy(paqueteSerializado + offset, &tamanioMensajes, medidaAMandar);
	offset = medidaAMandar;
	medidaAMandar = mensajes;
	memcpy(paqueteSerializado + offset, &mensajes, medidaAMandar);
	offset = medidaAMandar;
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

int informarPlanificadorLiberacionCPU(t_pcb* pcb,char* resultadosDeEjecuciones){
	uint32_t *totalPaquete = malloc(sizeof(uint32_t));
	char* pcbSerializado = serializarPCB(pcb, totalPaquete, resultadosDeEjecuciones);
	char* mensaje = malloc(*totalPaquete);
	memcpy(mensaje, pcbSerializado, *totalPaquete);
	int sendByte = send(socketPlanificador->fd, mensaje, *totalPaquete, 0);
	if (sendByte < 0) {
		log_error(CPULog, "Error al enviar el proc/pcb al Planificador","ERROR");
	}
	free(mensaje);
	free(pcbSerializado);
	free(totalPaquete);
	return EXIT_SUCCESS;
}

int informarEntradaSalida(uint32_t pid, int32_t tiempo, char* resultadosDeEjecuciones){
	int32_t status;
	int32_t cabecera = ENTRADA_SALIDA;
	uint32_t offset=0;
	char* mensaje = string_new();
	string_append(&mensaje,resultadosDeEjecuciones);
	string_append(&mensaje, "mProc ");
	string_append(&mensaje, string_itoa(pid));
	string_append(&mensaje, " en entrada-salida de tiempo ");
	string_append(&mensaje, string_itoa(tiempo));
	string_append(&mensaje, "\n");

	uint32_t longitudMensaje = strlen(mensaje);
	uint32_t tamanio = sizeof(cabecera) + sizeof(tiempo) + sizeof(uint32_t) + longitudMensaje;
	char* message2 = malloc(tamanio);
	memcpy(message2, &cabecera, sizeof(cabecera));
	offset = sizeof(cabecera);
	memcpy(message2, &tiempo, sizeof(tiempo));
	offset = offset + sizeof(tiempo);
	memcpy(message2 + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
	offset = offset + sizeof(uint32_t);
	memcpy(message2 + offset, mensaje, longitudMensaje);
	status = send(socketPlanificador->fd,message2,tamanio,0);
	free(message2);
	return "FIN";
}
