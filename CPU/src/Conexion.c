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
	//momentaneamente
	t_pcb pcbRecibido;

	sock_t* socketCliente = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);
	int32_t conexionPlanificador = connect_to_server(socketCliente);
	if (conexionPlanificador != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Planificador","ERROR");
		printf("NO se creo la conexion con planificador.\n");
	}
	printf("Se creo la conexion con planificador.\n");
	char message[1024];
	int status = 0;

	status = recv(socketCliente->fd, (void*)message, PAQUETE, 0); //Recibe mensaje de Planificador: PCB
	if(status >=0 ){
		//TODO Deserializar PCB
		pcbRecibido.path = "src/Prueba.txt";
		//return pcbRecibido;
		printf("Mensaje de Planificador: %s \n",message);
	}

	clean_socket(socketCliente);
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

	//TODO serializar estructura y enviar al AdminMemoria

	char* message = string_from_format("%s %s %s","Comando a ejecutar: iniciar", cantidadPaginas ," páginas.\n");
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
