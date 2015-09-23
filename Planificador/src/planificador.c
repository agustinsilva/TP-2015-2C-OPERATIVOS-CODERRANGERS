/***********PLANIFICADOR************/

#include "planificador.h"



int main(void)
{
	contadorProceso = 0;
	int respHilo = 0;
	int respServidor = 0;
	pthread_t hiloConsola;
	pthread_t hiloServidor;
	proc_listos = list_create();
	puts("Cargo archivo de configuracion de Planificador");
	cargarArchivoDeConfiguracion();
	respServidor = pthread_create(&hiloServidor,NULL,iniciarServidor,NULL);
	if(respServidor)
	{
		fprintf(stderr,"Error- Iniciar servidor codigo de retorn %d\n",respServidor);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}

	respHilo = pthread_create(&hiloConsola, NULL, mostrarConsola, NULL);
	if(respHilo)
	{
		fprintf(stderr,"Error - pthread_create() return code: %d\n",respHilo);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}


	pthread_join(hiloConsola, NULL);
	pthread_join(hiloServidor, NULL);

	puts("Fin de planificador");
	limpiarConfiguracion();
	return EXIT_SUCCESS;
}
