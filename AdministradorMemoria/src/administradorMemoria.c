/*********** AD
}MINISTRADOR MEMORIA ************/

#include "administradorMemoria.h"

int main(void) {

  	printf("Inicia el Administrador de Memoria \n");
	puts("Cargo archivo de configuración de Administrador Memoria\n");
	cargarArchivoDeConfiguracion();


	sock_t* servidor = create_server_socket(configuracion->puerto_escucha);
	listen_connections(servidor);
	sock_t* cpuSocket = accept_connection(servidor);
	printf("Esperando mensaje de Cpu \n");
	char mensajeCpu[1024];
	int32_t status;
	status = recv(cpuSocket->fd, (void*)mensajeCpu, 1024, 0);
	printf("Mensaje de cpu : %s \n",mensajeCpu);


	/*conecta con swap*/

	sock_t* clientSocketSwap = create_client_socket(configuracion->ip_swap,configuracion->puerto_swap);
	int32_t validationConnection = connect_to_server(clientSocketSwap);

	if (validationConnection != 0 ){
		printf("No se ha podido conectar correctamente al Swap\n");
	} else{
		printf("Se conectó al Swap\n");


		/* prepara mensaje para enviar */
		/*char* mensaje = "Hola Swap, soy el Admin de Memoria, mucho gusto ";*/
		int32_t status = enviarMensaje(clientSocketSwap,mensajeCpu);

		/*chequea envío*/
		if(!status){
			printf("No se envió el mensaje al swap\n");
		} else{
			printf("Se envió a Swap: %s\n", mensajeCpu);
		}

	/*recibe la respuesta*/
	char* respuesta = recibirMensaje(clientSocketSwap);
	printf("Recibe respuesta: %s\n", respuesta);
	free(respuesta);


	/* conecta con CPU : próximamente con hilos escucha*/
	sock_t* servidor = create_server_socket(configuracion->puerto_escucha);
	printf("Descriptor Servidor: %d \n", servidor->fd);
	listen_connections(servidor);
	printf("Escuchando conexiones de cpus... \n");
	sock_t* cpuSocket = accept_connection(servidor);
	printf("Esperando mensaje de Cpu \n");
	char message[1024];
	recv(cpuSocket->fd, (void*)message, 1024, 0);
	printf("Mensaje de cpu %s \n",message);

	printf("Finaliza Administrador de Memoria\n");
	}

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
