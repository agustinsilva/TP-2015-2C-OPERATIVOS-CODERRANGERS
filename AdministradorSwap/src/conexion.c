#include "administradorSwap.h"


//deprecado
char* recibirMensaje(sock_t* socket)
{
	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje;
	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	recv(socket->fd, &longitudMensaje, sizeof(int32_t), 0);
	char* mensaje = malloc(longitudMensaje+1);
	recv(socket->fd, mensaje, longitudMensaje, 0);
	mensaje[longitudMensaje]='\0';
	return mensaje;
}

int32_t enviarMensaje(sock_t* socket, char* mensaje)
{
	/*prepara la longitud del archivo a mandar, así el receptor sabe cuánto recibir*/
	int32_t longitud = string_length(mensaje);
	int32_t status = send(socket->fd, &longitud, sizeof(int32_t),0);
	/*chequea envío*/
	if(!status)
	{
		printf("No se envió la cantidad de bytes a enviar luego\n");
		return status;
	}
	status = send(socket->fd, mensaje, longitud,0);
	return status;
}


void iniciarServidor()
{
	uint32_t cabecera;
	t_mensaje* detalle;
	sock_t* socketServerSwap = create_server_socket(configuracion->puerto_escucha);
	listen_connections(socketServerSwap);
	printf("Escucha conexiones \n");
	sock_t* socketMemoria = accept_connection(socketServerSwap);
	printf("Administrador de memoria se ha conectado correctamente\n");
	while(1)
	{
		cabecera = deserializarEnteroSinSigno(socketMemoria);
		if(cabecera != ANORMAL)
		{
		detalle = deserializarDetalle(socketMemoria, cabecera);
		}
		switch (cabecera)
		{
			case INICIAR:
				procesarInicio(detalle,socketMemoria);
				break;
			case FINALIZAR:
				procesarFinalizacion(detalle,socketMemoria);
				break;
			case LEER:
				procesarLectura(detalle,socketMemoria);
				break;
			case ESCRIBIR:
				//Escribir falta implementar!
				break;
			case ANORMAL:
			printf("Finalizacion anormal de administrador de memoria\n");
			printf("Se finaliza el administrador de swap\n");
			clean_socket(socketServerSwap);
			clean_socket(socketMemoria);
			liberarRecursos();
			exit(1);
			break;
			default:
				// Por si acaso
				break;
		}
		free(detalle);
	}
	clean_socket(socketServerSwap);
	clean_socket(socketMemoria);
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

t_mensaje* deserializarDetalle(sock_t* socket, uint32_t cabecera)
{
	t_mensaje* detalle;
	detalle = malloc(sizeof(t_mensaje));
	switch (cabecera)
	{
		case INICIAR:
			printf("Se inicio un proceso\n");
			detalle->PID = deserializarEnteroSinSigno(socket);
			detalle->paginas = deserializarEnteroSinSigno(socket);
			break;
		case FINALIZAR:
			detalle->PID = deserializarEnteroSinSigno(socket);
			break;
		case LEER:
			detalle->PID = deserializarEnteroSinSigno(socket);
			detalle->ubicacion = deserializarEnteroSinSigno(socket);
			break;
		case ESCRIBIR:
			break;
		default:
			// Por si acaso
			break;
	}

	return detalle;
}
