/***********CPU************/

#include "cpu.h"

int main(void) {

	puts("Comienzo de cpu");
	puts("Cargo archivo de configuracion de CPU");
	CPULog = log_create("CPULog", "CPU", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	ConectarAPlanificador();
	puts("Fin de cpu \n");
	limpiarConfiguracion();
	log_destroy(CPULog);
	return EXIT_SUCCESS;
}
