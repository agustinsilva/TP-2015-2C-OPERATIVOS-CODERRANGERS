#include "cpu.h"

void updatePercentPerMin(){
	printf("PASARON 60 SEGUNDOS. Hora Actual: %s \n",temporal_get_string_time());
}

void iniciarCronTasks(){
	struct itimerval it;
	if (signal(SIGALRM, (void (*)(int)) updatePercentPerMin) == SIG_ERR) {
		perror("Unable to catch SIGALRM");
		exit(1);
	}
	it.it_value.tv_sec = 60;
	it.it_value.tv_usec = 0;
	it.it_interval=it.it_value;
	signal(SIGALRM, updatePercentPerMin);
	setitimer(ITIMER_REAL, &it, NULL);
}

double initTimes(time_t *tiempo1){
	double tiempo_inicio_instruccion = 0;
	tiempo_inicio_instruccion = time(tiempo1);
	return tiempo_inicio_instruccion;
}

/*dado un tiempo_inicio_instruccion calcula cuanto
 * tiempo transcurrió hasta ahora
*/
int calculateTimes(time_t *tiempo1, double tiempo_inicio_instruccion){
	time_t *tiempo2 = malloc(sizeof(time_t));
	double tiempo_fin_instruccion = 0;
	tiempo_fin_instruccion = time(tiempo2);
	int tiempo_transcurrido_instruccion = tiempo_fin_instruccion - tiempo_inicio_instruccion;
	free(tiempo1);
	free(tiempo2);
	return tiempo_transcurrido_instruccion;
}

void procesoDestroyer(t_CPUsConectados* cpu) {
	free(cpu);
}

void actualizarTiempoAcumuladoEjecucion(int tiempo_ejecucion_instruccion){
	int32_t pos = getPositionIfExists();
	if(pos!=-1){
		t_CPUsConectados *cpuOld = list_get(listaCPU,pos);
		//printf("va a remover el hilo nro: %u \n",cpuOld->idCPU);
		//ahora tengo q actualizarle el tiempo acumulado y volverlo a agregar a la lista
		t_CPUsConectados *cpu = malloc(sizeof(t_CPUsConectados));
		cpu->idCPU=cpuOld->idCPU;
		cpu->porcentajeProcesado=cpuOld->porcentajeProcesado;
		cpu->tiempoAcumuladoDeInstrucciones = cpuOld->tiempoAcumuladoDeInstrucciones + tiempo_ejecucion_instruccion;
		list_replace_and_destroy_element(listaCPU, (int)pos, (void*)cpu, (void*)procesoDestroyer);
		printf("[ESTADISTICAS] Se actualizó la lista de cpu, tiempo anterior: %d , tiempo transcurrido: %d ,"
				" ahora tiene: %d \n",cpuOld->tiempoAcumuladoDeInstrucciones,tiempo_ejecucion_instruccion,cpu->tiempoAcumuladoDeInstrucciones);
	}
}

int32_t getPositionIfExists(){
	uint32_t i;
	uint32_t rta = -1;
	uint32_t tamanio = list_size(listaCPU);
	t_CPUsConectados *cpu;
	pthread_t hilo = pthread_self();
	//printf("Nro de Hilo a buscar: %u\n",hilo);
	for( i=0 ; i < tamanio ; i++){
		cpu = list_get(listaCPU,i);
		//printf("iD de hilo: %u , en la pos: %u \n",cpu->idCPU,i);
		if(cpu->idCPU== hilo){
			rta = i;
		}
	}
	//printf("posicion (si es 9999 significa que no existe cpu con ese threadId): %d\n",rta);
	return rta;
}
