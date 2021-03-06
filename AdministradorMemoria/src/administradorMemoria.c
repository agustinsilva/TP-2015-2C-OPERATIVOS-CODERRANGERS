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

		if(servidor==NULL){
			log_error(MemoriaLog, "No se está pudiendo crear el socket servidor \n");
			limpiarRecursos();
			log_info(MemoriaLog, "Finaliza Administrador de Memoria\n");
			log_destroy(MemoriaLog);
			return EXIT_FAILURE;
		}

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
					list_add(CPUsConectados, cpuSocket);
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

	limpiarEstadisticas();
	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, CLOCKM)) {
		limpiarOrdenMarcos();
	}

	limpiarConfiguracion();
	limpiarMemoriaPrincipal();
	limpiarCPUs();
}

void setUp()
{
	if(configuracion->tlb_habilitada) {
		TLB = list_create();
	}

	estadisticas = list_create();
	t_Stats* tlb_stats = malloc(sizeof(t_Stats));
	tlb_stats->idProc = stat_TLB;
	tlb_stats->hit=0;
	tlb_stats->miss=0;
	tlb_stats->pagsTotales=0;
	tlb_stats->pageFaults=0;

	list_add(estadisticas,tlb_stats);

	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, CLOCKM)){
		ordenMarcos = list_create();
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
	CPUsConectados = list_create();

	iniciarCronTasks();

}

void iniciarCronTasks(){
	 struct itimerval it;

	 it.it_value.tv_sec = 1;

	 it.it_value.tv_usec = 0;
	 it.it_interval.tv_sec = 60;

	 it.it_interval.tv_usec = 0;
	 signal(SIGALRM, statsPerMinute);
	 setitimer(ITIMER_REAL, &it, NULL);
}

static void mpDestroyer(t_MP* entrada){
	free(entrada->contenido);
    free(entrada);
}

void TLBDestroyer(t_TLB* entrada) {
    free(entrada);
}

void ordenDestroyer(t_Orden* entrada) {
    free(entrada);
}

void marcosDestroyer(t_Marcos* entrada) {
	list_destroy_and_destroy_elements(entrada->marcos, (void*)ordenDestroyer);
    free(entrada);
}


void limpiarCPUs(){
	list_iterate(CPUsConectados, (void*) clean_socket);
	list_destroy(CPUsConectados);
}

void limpiarMemoriaPrincipal(){
	list_destroy_and_destroy_elements(memoriaPrincipal, (void*)mpDestroyer);
}

void limpiarTLB(){
	list_destroy_and_destroy_elements(TLB, (void*)TLBDestroyer);
}

void limpiarTablasDePaginas(){
	list_destroy_and_destroy_elements(tablasDePaginas, (void*)procesoDestroyer);
}

void limpiarOrdenMarcos(){
	list_destroy_and_destroy_elements(tablasDePaginas, (void*)marcosDestroyer);
}

void limpiarEstadisticas() {
	void statsDestroyer(t_Stats* entrada){
		free(entrada);
	}
	list_destroy_and_destroy_elements(estadisticas, (void*)statsDestroyer);
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
	pthread_mutex_init(&sem_stats, NULL);
	pthread_mutex_init(&sem_order, NULL);
}
