/*
 * gestionMemoria.c
 *
 *  Created on: 24/9/2015
 *      Author: ElianaLS
 */

#include "administradorMemoria.h"

/* --- Destroyers --- */

void procesoDestroyer(t_TP* entrada) {
    free(entrada);
}


/* --- Funciones Principales --- */

int32_t crearTablaDePaginas(int32_t idmProc, int32_t cantPaginas){

	if(hayEspacio()){

		if(!entraContiguo()) {

			/* comprimir */

		}

		/* generar entrada en tabla de paginas */
		int32_t i;
		for(i=0; i<cantPaginas; i++){
			t_TP* tabla = malloc(sizeof(t_TP));
			tabla->accessed = false;
			tabla->modified = false;
			tabla->present = false;
			tabla->read = true;
			tabla->write = false;

			tabla->idProc = idmProc;
			tabla->frame = getFrame();
			tabla->nroPag = getPagina();

			list_add(tablasDePaginas,tabla);
		}

		return pedido_exitoso;

	} else {

		return pedido_no_memoria;
	}
}

int32_t eliminarTablaDePaginas(int32_t idmProc){

	bool porIDProceso(void * entrada)
	{
		t_TP* tabla=(t_TP*) entrada;
		return tabla->idProc==idmProc;
	}
	t_TP* encontrado = list_find(tablasDePaginas,porIDProceso);
	if(encontrado==NULL){
		return pedido_error;
	}
	list_remove_and_destroy_by_condition(tablasDePaginas, porIDProceso, (void*) procesoDestroyer);
	return pedido_exitoso;
}


bool hayEspacio(){
	return true;
}

bool entraContiguo(){
	return true;
}

int32_t getFrame(){
	return rand() % 100;
}

int32_t getPagina(){
	return rand() % 100;
}

