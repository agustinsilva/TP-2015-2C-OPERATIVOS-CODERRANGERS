#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket.h"



/**** FUNCIONES PRIVADAS ****/

sock_t* _create_socket()
{
	sock_t* sock = (sock_t*) malloc(sizeof(sock_t));
	sock->sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr));

	sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock->fd==-1){
	      printf("Error en la creacion del socket, volviendo a intentar..");
	      sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	  }

	return sock;
}

void _prepare_conexion(sock_t* socket, char* ip, uint32_t puerto)
{
	// Seteamos la IP, si es NULL es localhost
	if(ip == NULL)
		socket->sockaddr->sin_addr.s_addr = htonl(INADDR_ANY); // Conexion Local
	else
		socket->sockaddr->sin_addr.s_addr = inet_addr(ip);	// Conexion Remota

	socket->sockaddr->sin_family = AF_INET;
	socket->sockaddr->sin_port = htons(puerto);
	memset(&(socket->sockaddr->sin_zero), '\0', 8);
}


int32_t _refresh_port(sock_t* socket)
{
	int32_t yes=1;
	return setsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t));
}


int32_t _bind_port(sock_t* socket)
{
	_refresh_port(socket);
	return bind(socket->fd, (struct sockaddr *)socket->sockaddr, sizeof(struct sockaddr));
}

int32_t _send(sock_t* socket, void* buffer, uint32_t len)
{
	return send(socket->fd,buffer,len,0);
}

int32_t _receive(sock_t* socket, void* buffer, uint32_t len)
{
	return recv(socket->fd, buffer, len, 0);
}

void _close_socket(sock_t* socket)
{
	shutdown(socket->fd, 2);
}

void _free_socket(sock_t* socket)
{
	free(socket->sockaddr);
	free(socket);
}

package_t* _create_package(void* data)
{
	package_t* package = malloc(sizeof(package_t));

	package->size = strlen(data);
	package->content = malloc(package->size);
	strcpy(package->content,data);

	return package;
}

int32_t _package_size(package_t* package)
{
	return package->size + sizeof(int32_t);
}

void* _serialize_package(package_t* package)
{
	char *serialized = malloc(_package_size(package)); //Malloc del tamaño a guardar

	int32_t offset = 0;
	int32_t size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(serialized + offset, &(package->size), size_to_send);
	offset += size_to_send;

	size_to_send = package->size;
	memcpy(serialized + offset, package->content, size_to_send);
	offset += size_to_send;

	return serialized;
}

package_t* _deserialize_message(char* serialized)
{
	u_int32_t size;
	int32_t offset = 0;
	package_t* package = malloc(sizeof(package_t));

	memcpy(&size, serialized, sizeof(u_int32_t));
	package->size = size;

	offset += sizeof(u_int32_t);
	package->content = malloc(package->size);
	memcpy(package->content, serialized + offset, package->size);

	return package;
}

int32_t _send_bytes(sock_t* socket, void* buffer, u_int32_t len)          //me aseguro que se envie toda la informacion
{
    int32_t total = 0;
    int32_t bytesLeft = len;
    int32_t n;
    while(total<len)
    {
        n = send(socket->fd, buffer+total, bytesLeft, 0);
        if(n==-1) break;
        total+=n;
        bytesLeft-=n;
    }
    len = total;
    return n==-1?-1:0;
}

int32_t _receive_bytes(sock_t* sock, void* bufferSalida, uint32_t lenBuffer)
{
	int32_t n = 0;
	int32_t bytesLeft = lenBuffer;
	int32_t recibido = 0;

	while(recibido < lenBuffer)
	{
		n = _receive(sock,bufferSalida+recibido,lenBuffer-recibido);
		if(n==-1 || n==0) break;
		recibido += n;
		bytesLeft -= n;
	}
	lenBuffer = recibido;
	return (n==-1 || n==0)?-1:0;
}


/**** FUNCIONES PUBLICAS ****/

/* Funcion para crear un socket servidor */

sock_t* create_server_socket(uint32_t puertoCliente)
{

	sock_t* sock = _create_socket();

	_prepare_conexion(sock,NULL,puertoCliente);

	if(_bind_port(sock) == -1)
			return NULL;

	return sock;
}

/* Funcion para crear un socket cliente */

sock_t* create_client_socket(char* ipServidor, uint32_t puertoServidor)
{

	sock_t* clientSocket = _create_socket();

	_prepare_conexion(clientSocket,ipServidor,puertoServidor);

	return clientSocket;

}

/* Conecta un cliente al socket servidor */

int32_t connect_to_server(sock_t* socket)
{
	return connect(socket->fd, (struct sockaddr *)socket->sockaddr, sizeof(struct sockaddr));
}

/* El socket servidor escucha conexiones de clientes entrantes */

int32_t listen_connections(sock_t* socket)
{
	return listen(socket->fd, CANTIDAD_CONEXIONES_LISTEN);
}

/* El socket servidor acepta conexion entrante */

sock_t* accept_connection(sock_t* socket)
{
	sock_t* sock_nuevo = _create_socket();

	uint32_t i = sizeof(struct sockaddr_in);

	sock_nuevo->fd = accept(socket->fd, (struct sockaddr *)sock_nuevo->sockaddr, &i);

	return sock_nuevo->fd == -1?NULL:sock_nuevo;
}

int32_t send_msg(sock_t* sock, void* msg)
{
	package_t* package = _create_package(msg);

	void* serialized = _serialize_package(package);

	int32_t serialized_lenght = _package_size(package);

	int32_t n = _send_bytes(sock, serialized, serialized_lenght);

	return n==-1?-1:0;
}

int32_t send_struct(sock_t* sock, void* pack, int32_t len_pack)
{
	int32_t n = send(sock->fd,pack,len_pack,0);

	return n==-1?-1:0;
}

package_t* receive_msg(sock_t* sock, void* output, int32_t len)
{
	int32_t n = _receive_bytes(sock,output,len);

	if(n==-1) return NULL;

	package_t* package = _deserialize_message(output);

	return package;
}

int32_t receive_struct(sock_t* sock, void* pack, int32_t len_pack)
{
	int32_t n = _receive_bytes(sock, pack, len_pack);

	if(n==-1) return NULL;

	return n;
}

void clean_socket(sock_t* sock)
{
	_close_socket(sock);
	_free_socket(sock);
}



