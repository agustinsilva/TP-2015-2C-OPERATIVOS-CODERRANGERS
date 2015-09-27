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
t_pcb escucharPlanificador(){
	char message[1024];
	int32_t status = 0;
	t_pcb pcbRecibido;
	t_stream stream;

	sock_t* socketClientePlanificador = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);
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
	//status = recv(socketClientePlanificador->fd,(void*)stream,sizeof(t_stream),0);

	/*if(status >=0 ){
		pcbRecibido = pcb_deserializar(stream);
	}*/

	clean_socket(socketClientePlanificador);
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
int informarAdminMemoriaComandoIniciar(char* cantidadPaginas){
	sock_t* socketAAdminMemoria = create_client_socket(configuracion->ipMemoria,configuracion->puertoMemoria);
	int32_t conexionAdminMemoria = connect_to_server(socketAAdminMemoria);
	if (conexionAdminMemoria != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Administrador de Memoria. \n","ERROR");
		//break;
		exit(EXIT_FAILURE);
	}

	//Envia aviso al Adm de Memoria: comando Iniciar.
	//enviarCodigoOperacion()

	//char* message = string_from_format("%s %s %s", INICIAR, cantidadPaginas ," páginas.\n");
	//int32_t status = send(socketAAdminMemoria->fd, (void*)message, strlen(message) + 1, 0);


	//TODO 	recibir la rta de memoria

	clean_socket(socketAAdminMemoria);
	return EXIT_SUCCESS;
}


/** Función que crea un socket para enviar la notificacion al Admin
 * de Memoria, cuando se solicitó el comando "finalizar"
 * @param id del proceso
 * @return int
 * 				0 Fallo
 * 				1 Exito
 */
int informarAdminMemoriaComandoFinalizar(char * path){
	sock_t* socketAAdminMemoria = create_client_socket(configuracion->ipMemoria,configuracion->puertoMemoria);
		int32_t conexionAdminMemoria = connect_to_server(socketAAdminMemoria);
		if (conexionAdminMemoria != 0) {
			perror("Error al conectar socket");
			log_error(CPULog,"Error al conectar CPU a Administrador de Memoria. \n","ERROR");
			//break;
			exit(EXIT_FAILURE);
		}

		//TODO serializar estructura y enviar al AdminMemoria

		char* message = string_from_format("%s %s %s","Comando a ejecutar: finalizar del mProc proveniente de: ", path ,"\n");
		int32_t status;
		status = send(socketAAdminMemoria->fd, (void*)message, strlen(message) + 1, 0);
		if(!status)	{
			printf("No se envió el mensaje al Administrador de Memoria.\n");
		}
		else {
			printf("Se envió el mensaje  correctamente al Admin de Memoria.\n");
		}

		clean_socket(socketAAdminMemoria);
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

/*t_pcb pcb_deserializar(t_stream stream){
	t_pcb *self = malloc (sizeof(t_pcb));
	int offset = 0, tmp_size = 0;

	memcpy (&self->idProceso, stream.data,tmp_size = sizeof(uint32_t));

	offset = tmp_size;
	for (tmp_size = 1; (stream.data + offset) [tmp_size-1] != '\0'; tmp_size++);
	self->estadoProceso =malloc (tmp_size);
	memcpy(&self->estadoProceso, stream.data + offset, tmp_size);

	offset += tmp_size;
	for (tmp_size = 1; (stream.data + offset ) [tmp_size-1] != '\0' ; tmp_size++);
	self->contadorPuntero= malloc (tmp_size);
	memcpy(&self->contadorPuntero, stream.data + offset, tmp_size);

	offset += tmp_size;
	for (tmp_size = 1; (stream.data + offset ) [tmp_size-1] != '\0' ; tmp_size++);
	self->cantidadInstrucciones= malloc (tmp_size);
	memcpy(&self->cantidadInstrucciones, stream.data + offset, tmp_size);

	offset += tmp_size;
	for (tmp_size = 1; (stream.data + offset ) [tmp_size-1] != '\0' ; tmp_size++);
	self->path= malloc (tmp_size);
	memcpy(self->path, stream.data + offset, tmp_size);

	return self;
}*/
