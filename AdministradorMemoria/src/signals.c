/*
 * signals.c
 *
 *  Created on: 18/10/2015
 *      Author: ElianaLS
 */

#include "administradorMemoria.h"


void finalizacion(){
	printf("\nSe intentó dar de baja el programa\n");
	printf("¿Seguro quiere darlo de baja? [S/N]");
	char respuesta;
	scanf("%c" ,&respuesta);
	if(respuesta=='s' || respuesta=='S'){
		exit(1);
	}
}

void TLBFlush(){
//	t_TLB* entrada1;
//	t_TLB* entrada2;
//	list_add(TLB, entrada1);
//	list_add(TLB, entrada2);

	printf("Tamaño TLB antes del flush: %d\n", list_size(TLB));
	bool all(t_TLB* entrada){
		return true;
	}
	list_remove_and_destroy_by_condition(TLB,(void*)all, (void*)TLBDestroyer);
	printf("Tamaño TLB después del flush: %d\n", list_size(TLB));
	log_info(MemoriaLog, "Se vació completamente la TLB \n");
}

void MPFush(){
	printf("Tiraste la señal para vaciar la memoria principal! \n No te voy a borrar nada hasta que"
			"no me digas cómo tiraste esta señal porque no encuentro nada en internet :(");
}

void MPDump(){
	printf("Tiraste la señal para dumpear la memoria principal! \n No te voy a copiar nada hasta que"
			"no me digas cómo tiraste esta señal porque no encuentro nada en internet :(");
}

void signalHandler(){
	if(signal(SIGINT, finalizacion) == SIG_ERR ) {
		printf("No se pudo atrapar la señal de finalización \n");
	}

	if(signal(SIGUSR1, TLBFlush) == SIG_ERR ) {
		printf("No se pudo atrapar la señal para limpiar la TLB \n");
	}

	if(signal(SIGUSR2, MPFush) == SIG_ERR ) {
			printf("No se pudo atrapar la señal para limpiar la memoria principal \n");
	}

	if(signal(SIGPOLL, MPDump) == SIG_ERR ) {
			printf("No se pudo atrapar la señal para dumpear la memoria principal \n");
	}
}
