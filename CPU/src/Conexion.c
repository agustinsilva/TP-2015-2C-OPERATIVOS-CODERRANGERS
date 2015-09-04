#include "cpu.h"

void* ConectarAPlanificador()
{
	struct sockaddr_in socketStruct;
		socketStruct.sin_family = AF_INET;
		socketStruct.sin_addr.s_addr = inet_addr(configuracion.ipPlanificador);
		socketStruct.sin_port = htons(configuracion.puertoPlanificador);
		memset(&(socketStruct.sin_zero), '\0', 8);

		int socketEmisor;
		// Crear un socket:
		// AF_INET: Socket de internet IPv4
		// SOCK_STREAM: Orientado a la conexion, TCP
		// 0: Usar protocolo por deft_paqueteecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
			if ((socketEmisor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("Error al crear socket");
				return EXIT_FAILURE;
			}
			// Conectar el socket con la direccion 'socketInfo'.
			if (connect(socketEmisor, (struct sockaddr*) &socketStruct, sizeof(socketStruct))	!= 0) {
				perror("Error al conectar socket");
				return EXIT_FAILURE;
			}
   printf("Se conecto a planificador");
   return NULL;
}
