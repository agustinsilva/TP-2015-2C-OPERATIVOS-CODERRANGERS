/*********** ADMINISTRADOR SWAP ************/

#include "administradorSwap.h"

int main(void)
{
	printf("Inicia Administrador de Swap\n");
	puts("Cargo archivo de configuracion de Administrador Swap\n");
	SwapLog = log_create("SwapLog", "AdministradorSwap", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	printf("Creando particion\n");
	inicializarParticion();
	tituloInicial();
	iniciarServidor();
	printf("Finaliza correctamente Administrador de Swap\n");
	liberarRecursos();
	return EXIT_SUCCESS;
}

void liberarRecursos()
{
	eliminarParticion();
	limpiarConfiguracion();
	log_destroy(SwapLog);
	list_destroy_and_destroy_elements(espacioLibre,(void*)limpiarNodosLibres);
	list_destroy_and_destroy_elements(espacioOcupado,(void*)limpiarNodosOcupados);
	list_destroy_and_destroy_elements(estadisticasProcesos,(void*)limpiarEstadisticas);
}

void limpiarNodosLibres(void* nodo)
{
	t_nodoLibre* nodoLibre = nodo;
	free(nodoLibre);
}

void limpiarNodosOcupados(void* nodo)
{
	t_nodoOcupado* nodoOcupado = nodo;
	free(nodoOcupado);
}

void limpiarEstadisticas(void* nodo)
{
	t_estadistica* estadistica = nodo;
	free(estadistica);
}

void tituloInicial(){

	printf("\n\t\t");


	printf("\e[48;5;196m \e[0m" "");
	printf("\e[48;5;196m \e[0m" "");
	printf("\e[48;5;202m \e[0m" "");
	printf("\e[48;5;202m \e[0m" "");
	printf("\e[48;5;208m \e[0m" "");
	printf("\e[48;5;208m \e[0m" "");
	printf("\e[48;5;214m \e[0m" "");
	printf("\e[48;5;214m \e[0m" "");
	printf("\e[48;5;226m \e[0m" "");
	printf("\e[48;5;226m \e[0m" "");
    printf("\e[48;5;229m \e[0m" "");
	printf("\e[48;5;229m \e[0m" "");
	printf("\e[48;5;229m \e[30m" BOLD "Administrador de swap" RESET_NON_BOLD "\e[0m");
	printf("\e[48;5;229m \e[0m" "");
	printf("\e[48;5;229m \e[0m" "");
	printf("\e[48;5;226m \e[0m" "");
	printf("\e[48;5;226m \e[0m" "");
	printf("\e[48;5;214m \e[0m" "");
    printf("\e[48;5;214m \e[0m" "");
	printf("\e[48;5;208m \e[0m" "");
	printf("\e[48;5;208m \e[0m" "");
	printf("\e[48;5;202m \e[0m" "");
	printf("\e[48;5;202m \e[0m" "");
	printf("\e[48;5;196m \e[0m" "");
	printf("\e[48;5;196m \e[0m" "");


	printf("\n\n");
}
