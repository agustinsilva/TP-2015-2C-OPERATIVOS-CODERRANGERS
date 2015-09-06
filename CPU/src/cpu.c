/***********CPU************/

#include "cpu.h"

int main(void) {

	puts("Cargo archivo de configuracion de CPU");
	cargarArchivoDeConfiguracion();
	ConectarAPlanificador();
	puts("Fin de cpu");
	return EXIT_SUCCESS;
}
