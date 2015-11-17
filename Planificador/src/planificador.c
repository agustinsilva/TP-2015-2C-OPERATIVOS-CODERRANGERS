/***********PLANIFICADOR************/
#include "planificador.h"

int main(void) {
	int respHilo = 0;
	int respServidor = 0;
	pthread_t hiloConsola;
	pthread_t hiloServidor;
	inicializar();
	puts("Cargo archivo de configuracion de Planificador");
	planificadorLog = log_create("PlanificadorLog", "Planificador", false, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	tituloInicial();
	respServidor = pthread_create(&hiloServidor, NULL, iniciarServidor, NULL);
	if (respServidor) {
		fprintf(stderr, "Error- Iniciar servidor codigo de retorno %d\n", respServidor);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}

	respHilo = pthread_create(&hiloConsola, NULL, mostrarConsola, NULL);
	if (respHilo) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", respHilo);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}

	pthread_join(hiloConsola, NULL);
	pthread_join(hiloServidor, NULL);

	puts("Fin de planificador");
	limpiarConfiguracion();
	return EXIT_SUCCESS;
}

void inicializar() {
	contadorProceso = 0;
	sem_init(&sincroproc, 0, 0);
	sem_init(&sincrocpu, 0, 0);
	sem_init(&sincroBloqueados, 0, 0);
	sem_init(&mutex, 0, 1);
	proc_listos = list_create();
	proc_ejecutados = list_create();
	proc_bloqueados = list_create();
	cpu_listos = list_create();
	cpu_ocupados = list_create();
	proc_metricas = list_create();
}

void tituloInicial(){
	int32_t i;
	printf("\n\t\t");
	for(i=40; i<45; i++){
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
	}
	printf("\e[48;5;45m" BOLD "\e[30mPlanificador" RESET_NON_BOLD "\e[0m");
	for(i=44; i>39; i--){
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
	}

	printf("\n\n");
}
