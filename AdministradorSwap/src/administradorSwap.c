/*********** ADMINISTRADOR SWAP ************/

#include "administradorSwap.h"

int main(void) {

	printf("Inicia Administrador de Swap\n");
	puts("Cargo archivo de configuracion de Administrador Swap\n");
	SwapLog = log_create("SwapLog", "AdministradorSwap", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	printf("Creando particion\n");
	inicializarParticion();
	iniciarServidor();
	printf("Finaliza Administrador de Swap\n");

	eliminarParticion();
	limpiarConfiguracion();
	log_destroy(SwapLog);
	return EXIT_SUCCESS;
}

