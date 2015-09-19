/*********** ADMINISTRADOR MEMORIA ************/

#include "administradorMemoria.h"

int main(void) {

	printf("Inicia el Administrador de Memoria \n");
	puts("Cargo archivo de configuración de Administrador Memoria\n");
	MemoriaLog = log_create("MemoriaLog", "AdministradorMemoria", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();

	/*conecta con swap*/

	sock_t* clientSocketSwap = create_client_socket(configuracion->ip_swap,configuracion->puerto_swap);
	int32_t validationConnection = connect_to_server(clientSocketSwap);
	if (validationConnection != 0 ){
		printf("No se ha podido conectar correctamente al Swap\n");
	} else{
		printf("Se conectó al Swap\n");

		sock_t* servidor = create_server_socket(configuracion->puerto_escucha);

		listen_connections(servidor);

		int32_t accept=1;

		while(accept){
			sock_t* cpuSocket = accept_connection(servidor);

			if (cpuSocket->fd!= -1){
				pthread_t hiloCPU;
				t_HiloCPU* paramsCPU = malloc(sizeof(t_HiloCPU));
				paramsCPU->cpuSocket = cpuSocket;
				paramsCPU->swapSocket = clientSocketSwap;

				if (pthread_create(&hiloCPU, NULL, hiloEjecucionCPU,(void*)paramsCPU)) {
					log_error(MemoriaLog,"Error al crear el hilo de CPU\n");
					EXIT_FAILURE;
				} else{
					log_info(MemoriaLog,"Conectado al hilo CPU de socket %d\n", cpuSocket->fd);
				}

			} else{
				accept=0;
			}

		}
		close(servidor);
		printf("Finaliza Administrador de Memoria\n");
	}
	limpiarConfiguracion();
	log_destroy(MemoriaLog);
	return EXIT_SUCCESS;
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

int32_t enviarMensaje(sock_t* socket, char* mensaje){

	/*prepara la longitud del archivo a mandar, así el receptor sabe cuánto recibir*/
	int32_t longitud = string_length(mensaje);
	int32_t status = send(socket->fd, &longitud, sizeof(int32_t),0);

	/*chequea envío*/
	if(!status){
		printf("No se envió la cantidad de bytes a enviar luego\n");
		return status;
	}
	status = send(socket->fd, mensaje, longitud,0);
	return status;
}

void* hiloEjecucionCPU(t_HiloCPU* paramsCPU){
	printf("Esperando mensaje de Cpu \n");
	char mensajeCpu[1024];
	int32_t status;
	status = recv(paramsCPU->cpuSocket->fd, (void*)mensajeCpu, 1024, 0);
	printf("Mensaje de cpu : %s \n",mensajeCpu);
	/* Deberia validar lo que recibio */
	/* prepara mensaje para enviar */
	/*char* mensaje = "Hola Swap, soy el Admin de Memoria, mucho gusto ";*/
	status = enviarMensaje(paramsCPU->swapSocket,mensajeCpu);

	/*chequea envío*/
	if(!status)	{
		printf("No se envió el mensaje al swap\n");
	}
	else {
		printf("Se envió a Swap: %s\n", mensajeCpu);
	}
	/*recibe la respuesta*/
	char* respuesta = recibirMensaje(paramsCPU->swapSocket);
	printf("Recibe respuesta: %s\n", respuesta);
	free(respuesta);
	return 0;
}

