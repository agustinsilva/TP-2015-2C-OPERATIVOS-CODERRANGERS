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


/* -------------------------------------------------------------------------------------*/

/* Funciones Principales */


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

	sock_t* clientSocketSwap = create_client_socket(configuracion->ip_swap,configuracion->puerto_swap);
	int32_t validationConnection = connect_to_server(clientSocketSwap);
	if (validationConnection != 0 ){
		printf("No se ha podido conectar correctamente al Swap\n");
	} else {
		printf("Se conectó al Swap\n");
		escribirPagsModificadas(clientSocketSwap);
	}
	actualizarTablaDePaginas();
	TLBFlush();
	vaciarMemoria();
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
