#include "planificador.h"

//falta optimizar con shared library sockets
void* iniciarServidor()
{
	fd_set set_maestro;
	fd_set set_temporal;
	uint32_t fdMaximo;
	int socketProcesado;
	int socketReceptor;
	int nuevoFd;
	FD_ZERO(&set_maestro);	//Limpia el set maestro
	FD_ZERO(&set_temporal); //Limpia el set temporal
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	struct sockaddr_storage remoteaddr; //dirección del cliente
	socklen_t addrlen;
	char* puertoEscuchaString;
	//revisar string_itoa
	puertoEscuchaString = string_itoa(configuracion.puertoEscucha);
	//Lleno la estructura de tipo addrinfo
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, puertoEscuchaString, &hints, &serverInfo);
	free(puertoEscuchaString);

	socketReceptor = socket(serverInfo->ai_family, serverInfo->ai_socktype,
				serverInfo->ai_protocol);

	int setOpcion = 1;
	if (setsockopt(socketReceptor, SOL_SOCKET, SO_REUSEADDR, &setOpcion, sizeof(int))
			== -1)
	{
			perror("setsockopt");
			exit(1);
	}
	bind(socketReceptor, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	listen(socketReceptor, 10);	// IMPORTANTE: listen() es una syscall BLOQUEANTE.

	// Agregar receptor al set maestro
	FD_SET(socketReceptor, &set_maestro);

	// Hacer seguimiento de descriptor maximo
	fdMaximo = socketReceptor;
	uint32_t status = 1;
	char paquete[PAQUETE];
    printf("Esperando conexiones");
	for (;;)
	{
		set_temporal = set_maestro;

		if (select(fdMaximo + 1, &set_temporal, NULL, NULL, NULL) == -1)
		{
		perror("select");
		exit(EXIT_SUCCESS);
		}

		//Recorre las conexiones existentes en busca de datos para leer
		for (socketProcesado = 0; socketProcesado <= fdMaximo; socketProcesado++)
		{
			if (FD_ISSET(socketProcesado, &set_temporal))
			{
				if (socketProcesado == socketReceptor) //Aca se recibe nueva conexion
				{
					addrlen = sizeof(remoteaddr);
					nuevoFd = accept(socketReceptor, (struct sockaddr *) &remoteaddr,
												&addrlen);
					if (nuevoFd == -1)
					{
					  printf("Error en accept\n");
					}
					else
					{
						FD_SET(nuevoFd, &set_maestro);
						if (nuevoFd > fdMaximo) //Maximo descriptor de socket
						{
							fdMaximo = nuevoFd;
						}
						printf("Se recibio nueva conexion\n");
					}
				}
				else //Aca se ejecuta el socket procesado
				{
					while (status != 0)
					{
					status = recv(socketProcesado, (void*) paquete, PAQUETE, 0);
					if (status != 0) printf("%s", paquete);
					}
				}
			}
		}
	}
return NULL;
}
