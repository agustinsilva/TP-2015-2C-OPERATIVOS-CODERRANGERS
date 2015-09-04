/***********CPU************/

#include "cpu.h"

int main(void) {

	puts("Cargo archivo de configuracion de CPU");
	cargarArchivoDeConfiguracion();
	ConectarAPlanificador();
	return EXIT_SUCCESS;
}
