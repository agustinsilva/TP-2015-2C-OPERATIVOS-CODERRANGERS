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

	liberarRecursos();
	return EXIT_SUCCESS;
}

void liberarRecursos()
{
	eliminarParticion();
	limpiarConfiguracion();
	log_destroy(SwapLog);
	list_destroy(espacioLibre);
	list_destroy(espacioOcupado);
	list_destroy(estadisticasProcesos);
}

