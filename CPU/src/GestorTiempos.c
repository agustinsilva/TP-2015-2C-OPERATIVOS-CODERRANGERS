#include "cpu.h"

void cpuDestroyer(t_CPUsConectados* cpu) {
	free(cpu);
}

/*Función que actualiza el porcentaje de Uso de todos los cpu's
 * del último minuto. Y vuelve a CERO el acumulador de cada cpu.
*/
void updatePercentPerMin()
{
	pthread_mutex_lock(&mutexListaCpus);
	//Hora Actual: temporal_get_string_time();
	log_info(CPULog,"Comienza Proceso de Actualización de Porcentajes de Uso de CPU'S.\n");
	uint32_t i;
	for( i=0 ; i < list_size(listaCPU) ; i++)
	{
		t_CPUsConectados* cpu = list_get(listaCPU,i);
		cpu->porcentajeProcesado = calculatePercent(cpu->tiempoAcumuladoDeInstrucciones);
		cpu->tiempoAcumuladoDeInstrucciones = 0;
		log_info(CPULog,"HILO de CPU nro: %u: Uso último min: %u%, Tiempo Acumulado"
				"(debe ser cero):%u% \n",cpu->idCPU,cpu->porcentajeProcesado,cpu->tiempoAcumuladoDeInstrucciones);
	}
	pthread_mutex_unlock(&mutexListaCpus);
}

int32_t calculatePercent(uint32_t tiempoAcumuladoDeInstrucciones)
{
	int32_t porcentaje = (tiempoAcumuladoDeInstrucciones * PORCENTAJE) / TIEMPO_MINUTO;
	if(porcentaje > PORCENTAJE)
	{
		porcentaje = PORCENTAJE;
	}
	return porcentaje;
}

void iniciarCronTasks(){
	struct itimerval it;
	if (signal(SIGALRM,updatePercentPerMin) == SIG_ERR) {
		perror("Unable to catch SIGALRM");
		exit(1);
	}
	it.it_value.tv_sec = TIEMPO_MINUTO;
	it.it_value.tv_usec = 0;
	it.it_interval = it.it_value;
	//signal(SIGALRM, updatePercentPerMin);
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
	time_t *tiempo2 =malloc(sizeof(time_t));
	double tiempo_fin_instruccion = time(tiempo2);
	int tiempo_transcurrido_instruccion = tiempo_fin_instruccion - tiempo_inicio_instruccion;
	free(tiempo1);
	free(tiempo2);
	return tiempo_transcurrido_instruccion;
}

void actualizarTiempoAcumuladoEjecucion(int tiempo_ejecucion_instruccion)
{
	int32_t pos = getPositionIfExists();
	if(pos!=-1)
	{
		t_CPUsConectados *cpu = list_get(listaCPU,pos);
		//ahora actualizo el tiempo acumulado
		cpu->tiempoAcumuladoDeInstrucciones = cpu->tiempoAcumuladoDeInstrucciones + tiempo_ejecucion_instruccion;
	}
}

int32_t getPositionIfExists(){
	uint32_t i;
	uint32_t rta = -1;
	uint32_t tamanio = list_size(listaCPU);
	t_CPUsConectados *cpu;
	pthread_t hilo = pthread_self();
	for( i=0 ; i < tamanio ; i++){
		cpu = list_get(listaCPU,i);
		if(cpu->idCPU== hilo){
			rta = i;
		}
	}
	//posicion (si es 9999 significa que no existe cpu con ese threadId): %d\n",rta);
	return rta;
}
