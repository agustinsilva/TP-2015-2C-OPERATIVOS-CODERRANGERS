/*
 * signals.c
 *
 *  Created on: 18/10/2015
 *      Author: ElianaLS
 */

#include "administradorMemoria.h"


/* Funciones Secundarias */

void escribirPagsModificadas(sock_t* socketSwap){

	bool modificadasEnMP(t_TP* entrada){
		return entrada->present==true && entrada->modified==true;
	}
	t_list* pagsAEscribir = list_filter(tablasDePaginas, (void*) modificadasEnMP);

	void escribirModificadas(t_TP* entrada){
		if(escribirEnSwap(entrada, socketSwap)){
			log_info(MemoriaLog, "Se escribió la página %d del proceso %d antes de limpiar la memoria.\n", entrada->nroPag, entrada->idProc);
		} else {
			log_error(MemoriaLog,RED "No se pudo guardar la página %d del proceso %d en la partición antes de vaciar la memoria. Se perderá el contenido.\n"RESET, entrada->nroPag, entrada->idProc);
		}
		entrada->modified = false;
	}
	list_iterate(pagsAEscribir, (void*)escribirModificadas);
}

void actualizarTablaDePaginas(){

	void sacarDeMP(t_TP* entrada){
		entrada->present = false;
		entrada->frame = marco_inhabilitado;
		entrada->accessed = false;
		entrada->loadedTime = REINIT;
		entrada->usedTime = REINIT;
	}
	list_iterate(tablasDePaginas, (void*)sacarDeMP);

}

void vaciarMemoria(){
	void vaciarMP(t_MP* entrada){
		entrada->ocupado = false;
		memcpy(entrada->contenido, "",configuracion->tamanio_marco);
	}
	list_iterate(memoriaPrincipal, (void*)vaciarMP);
}

void doMPFlush(sock_t* socketSwap){

	pthread_mutex_lock(&sem_TP);
	pthread_mutex_lock(&sem_MP);
	pthread_mutex_lock(&sem_swap);

	if (clientSocketSwap == 0 ){
		log_info(MemoriaLog, "Se perdió la conexión con el Administrador de Swap\n");
		pthread_mutex_unlock(&sem_swap);
		pthread_mutex_unlock(&sem_MP);
		pthread_mutex_unlock(&sem_TP);
		return;
	} else {
		log_info(MemoriaLog,"Se escribirán las páginas modificadas\n");
		escribirPagsModificadas(socketSwap);
	}

	actualizarTablaDePaginas();

	if(configuracion->tlb_habilitada){
		doTLBFlush();
	}

	vaciarMemoria();

	pthread_mutex_unlock(&sem_swap);
	pthread_mutex_unlock(&sem_MP);
	pthread_mutex_unlock(&sem_TP);
}

void doTLBFlush(){

	if(configuracion->tlb_habilitada){
		pthread_mutex_lock(&sem_TLB);
		printf("Tamaño TLB antes del flush: %d\n", list_size(TLB));
		list_clean_and_destroy_elements(TLB, (void*)TLBDestroyer);
		printf("Tamaño TLB después del flush: %d\n", list_size(TLB));
		pthread_mutex_unlock(&sem_TLB);

		log_info(MemoriaLog, "Se vació completamente la TLB \n");
	}else{
		log_info(MemoriaLog, "No se puede realizar TLB Flush: La TLB no está habilitada\n");
	}

}


void printearTabla(){
	pthread_mutex_lock(&sem_MP);
	int32_t index=0;
	void printear(t_MP* entrada){
		if(index==0){
			printf(" Marco \t| Contenido\t\n");
			printf("---------------------------------------\n");
		}
		printf("   %d\t| %s\t|\n", entrada->marco, entrada->contenido);
		index++;
	}
	list_iterate(memoriaPrincipal, (void*)printear);
	pthread_mutex_unlock(&sem_MP);
}

/* -------------------------------------------------------------------------------------*/

/* Funciones Principales */


void finalizacion(){
	printf("\nSe intentó dar de baja el programa\n");
	printf("¿Seguro quiere darlo de baja? [S/N]");
	char respuesta;
	scanf("%c" ,&respuesta);
	if(respuesta=='s' || respuesta=='S'){
		clean_socket(clientSocketSwap);
		exit(1);
	}
}


void TLBFlush(){
	pthread_t hiloTLBFlush;
	if (pthread_create(&hiloTLBFlush, NULL,(void*)doTLBFlush,NULL)){
		log_error(MemoriaLog,RED"Error al crear el hilo para realizar TLBFlush\n"RESET);
	}
//	pthread_join(hiloTLBFlush,NULL);
}

void MPFlush(){
	pthread_t hiloMPFlush;
	if (pthread_create(&hiloMPFlush, NULL,(void*)doMPFlush,NULL)){
		log_error(MemoriaLog,RED"Error al crear el hilo para realizar MemFlush\n"RESET);
	}
//	pthread_join(hiloMPFlush,NULL);
}

void MPDump(){

	int pid;
	pid = fork();
	if (pid < 0) {
		log_error(MemoriaLog,RED"Error al crear proceso hijo para realizar Dump de Memoria Principal\n"RESET);
		return;
	}
	else if (pid == 0) {
		/* Inicio de código de Proceso Hijo */
		sleep(5);
		printf("Soy un hijo de frula\n");
		sleep(5);

		printearTabla();
		/* Fin de código de Proceso Hijo */
	}
//	else {
//		/* Inicio de código de Proceso Padre */
//		printf("Tiraste la señal para dumpear la memoria principal! \n ");
//
//		/* Esperar finalización del Hijo */
//		wait(NULL);
//		printf("Terminó mi hijo \n");
//		/* Fin de código de Proceso Padre */
//	}

}


void signalHandler(){
	if(signal(SIGINT, finalizacion) == SIG_ERR ) {
		printf("No se pudo atrapar la señal de finalización \n");
	}

	if(signal(SIGUSR1, TLBFlush) == SIG_ERR ) {
		printf("No se pudo atrapar la señal para limpiar la TLB \n");
	}

	if(signal(SIGUSR2, MPFlush) == SIG_ERR ) {
			printf("No se pudo atrapar la señal para limpiar la memoria principal \n");
	}

	if(signal(SIGPOLL, MPDump) == SIG_ERR ) {
			printf("No se pudo atrapar la señal para dumpear la memoria principal \n");
	}
}
