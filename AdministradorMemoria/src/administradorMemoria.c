/*********** ADMINISTRADOR MEMORIA ************/

#include "administradorMemoria.h"

int main(void)
{
	MemoriaLog = log_create("MemoriaLog", "AdministradorMemoria", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	initializeMutex();
	signalHandler();
	saludoInicial();

	setUp();

	sleep(126);

	/*conecta con swap*/
	clientSocketSwap = create_client_socket(configuracion->ip_swap,configuracion->puerto_swap);
	int32_t validationConnection = connect_to_server(clientSocketSwap);
	if (validationConnection != 0 ) {
		log_error(MemoriaLog, RED"No se ha podido conectar correctamente al Swap\n"RESET);
		clean_socket(clientSocketSwap);
	}
	else
	{
		log_info(MemoriaLog,"Se conectó al Swap\n");
		sock_t* servidor = create_server_socket(configuracion->puerto_escucha);
		listen_connections(servidor);
		int32_t accept=1;
		while(accept)
		{
			sock_t* cpuSocket = accept_connection(servidor);
			if (cpuSocket->fd!= -1)
			{
				pthread_t hiloCPU;
				t_HiloCPU* paramsCPU = malloc(sizeof(t_HiloCPU));
				paramsCPU->cpuSocket = cpuSocket;
				paramsCPU->swapSocket = clientSocketSwap;
				if (pthread_create(&hiloCPU, NULL,(void*)hiloEjecucionCPU,(void*)paramsCPU)) {
					log_error(MemoriaLog,RED"Error al crear el hilo de CPU\n"RESET);
					return EXIT_FAILURE;
				}
				else {
					log_info(MemoriaLog,"Conectado al hilo CPU de socket %d", cpuSocket->fd);
				}
			}
			else {
				accept=0;
			}

		}
		clean_socket(servidor);
	}
	limpiarRecursos();
	log_info(MemoriaLog, "Finaliza Administrador de Memoria\n");
	log_destroy(MemoriaLog);
	return EXIT_SUCCESS;
}

void limpiarRecursos()
{
	if(configuracion->tlb_habilitada){
		limpiarTLB();
	}
	limpiarConfiguracion();
	limpiarMemoriaPrincipal();
}

void setUp()
{
	if(configuracion->tlb_habilitada)
	{
		TLB = list_create();
	}
	memoriaPrincipal = list_create();
	int32_t i;
	for(i=0; i<configuracion->cantidad_marcos; i++)
	{
		t_MP* entrada = malloc(sizeof(t_MP));
		entrada->marco=i;
		entrada->ocupado = false;
		entrada->contenido = malloc(configuracion->tamanio_marco);
		llenarDeNulos(entrada->contenido,configuracion->tamanio_marco,0);

		list_add(memoriaPrincipal,entrada);
	}
	tablasDePaginas = list_create();
}

static void mpDestroyer(t_MP* entrada)
{
	free(entrada->contenido);
    free(entrada);
}

void TLBDestroyer(t_TLB* entrada)
{
    free(entrada);
}

void limpiarMemoriaPrincipal()
{
	list_destroy_and_destroy_elements(memoriaPrincipal, (void*)mpDestroyer);
}
void limpiarTLB()
{
	list_destroy_and_destroy_elements(TLB, (void*)TLBDestroyer);
}
void limpiarTablasDePaginas()
{
	list_destroy_and_destroy_elements(tablasDePaginas, (void*)procesoDestroyer);
}
void saludoInicial(){

	int32_t i;
	printf("\n\t\t");
	for(i=16; i<21; i++){
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
	}

	printf("\e[48;5;21m" BOLD "Administrador de Memoria" RESET_NON_BOLD "\e[0m");
	for(i=21; i>16; i--){
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
	}

	printf("\n\n");
}

void initializeMutex(){
	pthread_mutex_init(&sem_TLB, NULL);
	pthread_mutex_init(&sem_TP, NULL);
	pthread_mutex_init(&sem_MP, NULL);
	pthread_mutex_init(&sem_swap, NULL);
}
