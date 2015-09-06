/***********CPU************/

#include "cpu.h"

int main(void) {

	puts("Comienzo de cpu");
	puts("Cargo archivo de configuracion de CPU");
	cargarArchivoDeConfiguracion();
	ConectarAPlanificador();
	puts("Fin de cpu \n");
	return EXIT_SUCCESS;
}
