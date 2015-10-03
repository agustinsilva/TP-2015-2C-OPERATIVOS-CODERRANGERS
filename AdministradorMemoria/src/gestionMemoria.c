/*
 * gestionMemoria.c
 *
 *  Created on: 24/9/2015
 *      Author: ElianaLS
 */

#include "administradorMemoria.h"

/* --- Destroyers --- */

void procesoDestroyer(t_TP* entrada)
{
	free(entrada);
}


/* --- Funciones Principales --- */

int32_t crearTablaDePaginas(int32_t idmProc, int32_t cantPaginas)
{
	/* generar entrada en tabla de paginas */
	int32_t i;
	for(i=0; i<cantPaginas; i++)
	{
		t_TP* tabla = malloc(sizeof(t_TP));
		tabla->accessed = false;
		tabla->modified = false;
		tabla->present = false;
		tabla->idProc = idmProc;
		/*los frames no se asignan aun*/
		tabla->nroPag = getPagina();
		list_add(tablasDePaginas,tabla);
	}
	return pedido_exitoso;
}

int32_t eliminarTablaDePaginas(int32_t idmProc)
{
	bool porIDProceso(void * entrada)
	{
		t_TP* tabla=(t_TP*) entrada;
		return tabla->idProc==idmProc;
	}
	t_TP* encontrado = list_find(tablasDePaginas,porIDProceso);
	if(encontrado==NULL)
	{
		return pedido_error;
	}
	list_remove_and_destroy_by_condition(tablasDePaginas, porIDProceso, (void*) procesoDestroyer);
	return pedido_exitoso;
}

bool hayEspacio()
{
	return true;
}

int32_t getFrame()
{
	int32_t marcoLibre=-1;
	void buscarMarcoLibre(t_MP* entrada)
	{
		if(entrada->pagina==-1)
		{
			marcoLibre=entrada->marco;
			return;
		}
	}
	list_iterate(memoriaPrincipal, (void*)buscarMarcoLibre);
	return marcoLibre;
}

int32_t getPagina(t_list* tablasDePaginas, int32_t idMProc)
{
	int32_t pagina =  rand() % 1000;
	bool porProcesoYNroPagina(void * entrada)
	{
		t_TP* tabla=(t_TP*) entrada;
		return tabla->nroPag==pagina && tabla->idProc==idMProc;
	}
	t_TP* encontrado = list_find(tablasDePaginas,porProcesoYNroPagina);
	while(encontrado!=NULL)
	{
		pagina =  rand() % 1000;
		bool porProcesoYNroPagina(void * entrada)
		{
			t_TP* tabla=(t_TP*) entrada;
			return tabla->nroPag==pagina && tabla->idProc==idMProc;
		}
		encontrado = list_find(tablasDePaginas,porProcesoYNroPagina);
	}
	return pagina;
}

