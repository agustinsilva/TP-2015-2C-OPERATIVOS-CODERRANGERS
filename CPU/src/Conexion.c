#include "cpu.h"

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
	char message[1024];
	int32_t status = 0;
	t_pcb* pcbRecibido = malloc(sizeof(t_pcb));
	sock_t* socketClientePlanificador = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);
	socketPlanificador = socketClientePlanificador;

	int32_t conexionPlanificador = connect_to_server(socketClientePlanificador);
	if (conexionPlanificador != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Planificador","ERROR");
		printf("NO se creo la conexion con planificador.\n");
	}
	printf("Se creo la conexion con planificador.\n");

	//Envia aviso al Plani de que se creó un nuevo hilo cpu.
	enviarCodigoOperacion(socketClientePlanificador,NUEVO_HILO);

	//Recibe mensaje de Planificador: PCB
	uint32_t tamanioChar;
	status = recv(socketClientePlanificador->fd,&(pcbRecibido->idProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketClientePlanificador->fd,&(pcbRecibido->estadoProceso),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketClientePlanificador->fd,&(pcbRecibido->contadorPuntero),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketClientePlanificador->fd,&(pcbRecibido->cantidadInstrucciones),sizeof(uint32_t),0);
	if (status <= 0) log_error(CPULog,"Error al recibir PCB.","ERROR");
	status = recv(socketClientePlanificador->fd,&(tamanioChar),sizeof(uint32_t),0);
	pcbRecibido->path = malloc(tamanioChar);
	status = recv(socketClientePlanificador->fd,pcbRecibido->path,tamanioChar,0);
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
int informarAdminMemoriaComandoIniciar(char* cantidadPaginas, int32_t pid){
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

	cabecera = RESPUESTA_PLANIFICADOR;
	char* mensaje = string_new();
	offset = 0;
	if(status==PEDIDO_ERROR){//mProc X - Fallo
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Fallo.");
		uint32_t tamanio = sizeof(cabecera) + sizeof(mensaje);
		message = malloc(tamanio);
		memcpy(message, &cabecera, sizeof(cabecera));
		offset = sizeof(cabecera);
		memcpy(message + offset, &mensaje, sizeof(mensaje));
		offset = offset + sizeof(mensaje);
		status = send(socketPlanificador->fd,message,tamanio,0);
		free(message);
	}else{//mProc X - Iniciado
		string_append(&mensaje, "mProc ");
		string_append(&mensaje, string_itoa(pid));
		string_append(&mensaje, " - Iniciado.");
		uint32_t longitudMensaje = strlen(mensaje);
		uint32_t tamanio = sizeof(cabecera) + sizeof(uint32_t) + longitudMensaje;
		message = malloc(tamanio);
		memcpy(message, &cabecera, sizeof(cabecera)); //Codigo
		offset = sizeof(cabecera);
		memcpy(message + offset, &longitudMensaje, sizeof(uint32_t)); //longitud mensaje
		offset = offset + sizeof(uint32_t);
		memcpy(message + offset, mensaje, longitudMensaje); //Mensaje
		offset = offset + longitudMensaje;
		status = send(socketPlanificador->fd,message,tamanio,0);
		free(message);
	}
	return EXIT_SUCCESS;
}


/** Función que crea un socket para enviar la notificacion al Admin
 * de Memoria, cuando se solicitó el comando "finalizar"
 * @param id del proceso
 * @return int
 * 				0 Fallo
 * 				1 Exito
 */
int informarAdminMemoriaComandoFinalizar(int32_t pid){
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
	string_append(&mensaje, "mProc ");
	string_append(&mensaje, string_itoa(pid));
	string_append(&mensaje, " - Finalizado.");
	uint32_t longitudMensaje = strlen(mensaje);
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

	return EXIT_SUCCESS;
}

int informarAdminMemoriaComandoLeer(int32_t pid, char* pagina){
	int32_t status;
	int32_t entero;
	int32_t numeroPagina = atoi(pagina);
	//Envia aviso al Adm de Memoria: comando Finalizar.
	int32_t cabecera = FINALIZAR;
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

	//Recibe respuesta
	//status = recv(socketAdminMemoria->fd,&entero,sizeof(int32_t),0);
	uint32_t codOperacion = deserializarEnteroSinSigno(socketAdminMemoria);
	uint32_t longitud = deserializarEnteroSinSigno(socketAdminMemoria);
	char* contenido;
	status = recv(socketAdminMemoria->fd,&contenido,longitud,0);

	//mProc 10 - Pagina 2 leida: contenido
	cabecera = RESPUESTA_PLANIFICADOR;
	char* mensaje = string_new();
	offset = 0;
	string_append(&mensaje, "mProc ");
	string_append(&mensaje, string_itoa(pid));
	string_append(&mensaje, " - Pagina ");
	string_append(&mensaje,numeroPagina);
	string_append(&mensaje," leida: ");
	string_append(&mensaje,contenido);
	uint32_t longitudMensaje = strlen(mensaje);
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

	return EXIT_SUCCESS;
}

void enviarCodigoOperacion(sock_t* socket, int32_t entero){
	int32_t enviado = send(socket->fd, &entero, sizeof(int32_t), 0);
	if(enviado!=sizeof(int32_t)){
		printf("No se envió correctamente la información entera\n");
		log_error(CPULog,"Error al enviar codigo de operacion. \n","ERROR");
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


