#include "cpu.h"

void* ConectarAPlanificador()
{
	sock_t* socketCliente = create_client_socket(configuracion.ipPlanificador,configuracion.puertoPlanificador);

	int32_t resultadoConexion = connect_to_server(socketCliente);
	if (resultadoConexion != 0) {
		perror("Error al conectar socket");
		return (void*)EXIT_FAILURE;
	}
	int32_t resultadoEnvio;
	char message[1024];
	int enviar = 1;
	while(enviar)
	{
	fgets(message, 1024, stdin);
	if (!strcmp(message,"exit\n")) enviar = 0;
	if (enviar){
	send_msg(socketCliente,message);
	}
	}
	clean_socket(socketCliente);
   return NULL;
}
