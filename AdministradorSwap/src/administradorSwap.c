/*********** ADMINISTRADOR SWAP ************/

#include "administradorSwap.h"

int main(void) {

	printf("Inicia Administrador de Swap\n");
	puts("Cargo archivo de configuracion de Administrador Swap\n");
	SwapLog = log_create("SwapLog", "AdministradorSwap", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();

	printf("Creando particion\n");
	crearParticion();
	printf("Particion creada con exito\n");
	sock_t* socketServerSwap = create_server_socket(configuracion->puerto_escucha);
	listen_connections(socketServerSwap);
	printf("Escucha conexiones \n");

	sock_t* socketCliente = accept_connection(socketServerSwap);
	printf("Conexión: %d\n", socketCliente->fd);

	char* mensaje = recibirMensaje(socketCliente);

	printf("Mensaje de Admin memoria: %s\n", mensaje);
	free(mensaje);


	 //envia mensaje
	char* respuesta = "Hola Memoria, un gusto.";
	int32_t status = enviarMensaje(socketCliente,respuesta);

	//chequea envío
	if(!status){
		printf("No se envió el mensaje al swap\n");
	} else{
		printf("Se envió a Memoria: %s\n", respuesta);
	}

	printf("Finaliza Administrador de Swap\n");
	eliminarParticion();
	limpiarConfiguracion();
	log_destroy(SwapLog);
	return EXIT_SUCCESS;
}

char* recibirMensaje(sock_t* socket){

	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje;

	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	recv(socket->fd, &longitudMensaje, sizeof(int32_t), 0);

	char* mensaje = malloc(longitudMensaje+1);

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

void crearParticion()
{
	char instruccion[1000]={0};
	sprintf(instruccion, "dd if=/dev/zero of=%s bs=%d count=%d", configuracion->nombre_swap,configuracion->tamano_pagina,configuracion->cantidad_paginas);
	system(instruccion);
}

void eliminarParticion()
{
	if (remove(configuracion->nombre_swap) == 0){
		printf("Elimino correctamente la particion \n");
	}
	else{
		printf("No se elimino correctamente la particion \n");
	}
}
