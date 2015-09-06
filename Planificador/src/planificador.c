/***********PLANIFICADOR************/

#include "planificador.h"

int main(void)
{
	puts("Cargo archivo de configuracion de Planificador");
	cargarArchivoDeConfiguracion();
	iniciarServidor();
	puts("Fin de planificador");
	return EXIT_SUCCESS;
}

