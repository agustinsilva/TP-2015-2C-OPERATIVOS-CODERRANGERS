#include "planificador.h"

//falta optimizar con shared library sockets
void* iniciarServidor()
{
	fd_set set_maestro,set_temporal;
	uint32_t fdMaximo,socketProcesado,socketReceptor,nuevoFd;
	FD_ZERO(&set_maestro);	//Limpia el set maestro
	FD_ZERO(&set_temporal); //Limpia el set temporal
	struct sockaddr_storage remoteaddr; //dirección del cliente
	socklen_t addrlen;
	socketReceptor = crearSocketReceptor();
	listen(socketReceptor, 10);	// IMPORTANTE: listen() es una syscall BLOQUEANTE.
	// Agregar receptor al set maestro
	FD_SET(socketReceptor, &set_maestro);
	// Hacer seguimiento de descriptor maximo
	fdMaximo = socketReceptor;
	uint32_t status = 1;
	char paquete[PAQUETE];
	int enviar = 1;
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
						printf("Mandar mensaje a CPU\n");
						printf("Ingresar mensaje\n");
						fgets(paquete, PAQUETE, stdin);
						send(nuevoFd, paquete, strlen(paquete) + 1, 0);

					}
				}
				else //Aca se ejecuta el socket procesado
				{

					status = recv(socketProcesado, (void*)paquete, PAQUETE, 0);
					if (status > 0)
					{
						printf("%s", paquete);
					}
					if(status <= 0){
						puts("Se desconecto cliente \n");
						close(socketProcesado);
						FD_CLR(socketProcesado,&set_maestro);
						return NULL;
					}

				}
			}
		}
	}
return NULL;
}


uint32_t crearSocketReceptor()
{
	uint32_t socketReceptor;
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	char* puertoEscuchaString;
	puertoEscuchaString = string_itoa(configuracion->puertoEscucha);
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
	return socketReceptor;
}