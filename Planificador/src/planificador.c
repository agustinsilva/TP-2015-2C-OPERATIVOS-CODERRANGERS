/***********PLANIFICADOR************/

#include "planificador.h"

int main(void)
{
	puts("Cargo archivo de configuracion de Planificador");
	cargarArchivoDeConfiguracion();
	iniciarServidor();
	return EXIT_SUCCESS;
}

