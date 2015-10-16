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
		tabla->nroPag = i;
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
		if(entrada->ocupado==0) {
			marcoLibre=entrada->marco;
			return;
		}
	}
	list_iterate(memoriaPrincipal, (void*)buscarMarcoLibre);
	return marcoLibre;
}

/* unico reemplazo -> FIFO */
void actualizarTLB(int32_t idmProc, int32_t nroPagina, int32_t marco){
	if(list_size(TLB)>=configuracion->entradas_tlb){
		list_remove_and_destroy_element(TLB,0,(void*) TLBDestroyer);
	}
	t_TLB* nuevaEntrada = malloc(sizeof(t_TLB));
	nuevaEntrada->idProc = idmProc;
	nuevaEntrada->pagina = nroPagina;
	nuevaEntrada->marco = marco;

	list_add(TLB, nuevaEntrada);
}


void actualizarTLBSwap(int32_t idmProc, int32_t nroPagina, int32_t marco){

	/* reviso la TLB */
	int32_t indexEnTabla=-1, index;
	for(index=0; index<list_size(TLB); index++){
		t_TLB* entradaTLB = list_get(TLB,index);
		if(entradaTLB->marco==marco){
			indexEnTabla=index;
		}
	}

	/* si hay una entrada con el marco actualizado, actualizo*/
	if(indexEnTabla!=-1){

		t_TLB* entradaAActualizar = list_remove(TLB,indexEnTabla);
			if(entradaAActualizar!=NULL){
				entradaAActualizar->idProc=idmProc;
				entradaAActualizar->pagina=nroPagina;
				entradaAActualizar->marco=marco;
			}
		list_add(TLB,entradaAActualizar);
	}

}

int32_t swapIN(sock_t* swapSocket, sock_t* cpuSocket, int32_t idmProc, int32_t nroPagina){
	t_LecturaSwap* pedido = pedirPagina(swapSocket,idmProc, nroPagina);
	if(pedido==NULL || pedido->encontro==false)
	{
		enviarEnteros(cpuSocket, pedido_error);
		return -1;
	}
	log_info(MemoriaLog, " - *Acceso a SWAP*  PID: %d", idmProc);


	int32_t marcoAReemplazar;
	int32_t cantMarcosOtorgados = calcularCantPaginasEnMP(idmProc);
	if(cantMarcosOtorgados>=configuracion->cantidad_marcos){
		/* Swap Out*/
		// marcoAReemplazar = reemplazarMP(idmProc, nroPagina, configuracion->algoritmo_reemplazo);
	} else {
		marcoAReemplazar = getRandomFrameVacio();
	}

	t_MP* mp = actualizarMP(idmProc, nroPagina, marcoAReemplazar, pedido);

	enviarContenidoPagina(cpuSocket, pedido);
	free(pedido->contenido);
	free(pedido);

	return mp->marco;
}


t_MP* actualizarMP(int32_t idmProc, int32_t nroPagina, int32_t marcoAReemplazar, t_LecturaSwap* pedido){

	/* saco la página de MP y le inhabilito el marco*/
	bool porMarco(t_TP* entrada){
			return entrada->frame==marcoAReemplazar;
	}
	t_TP* paginaSwappedOut = list_find(tablasDePaginas, (void*) porMarco);
	paginaSwappedOut->present = false;
	paginaSwappedOut->frame = -1;

	/* llevo la página a MP y le asigno el marco otorgado*/
	bool porPIDyPag(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->nroPag==nroPagina;
	}
	t_TP* paginaSwappedIn = list_find(tablasDePaginas, (void*) porPIDyPag);
	paginaSwappedIn->present=true;
	paginaSwappedIn->frame = marcoAReemplazar;

	/* por las dudas le pongo el ocupado */
	t_MP* mp = buscarEnMemoriaPrincipal(paginaSwappedIn->frame);
	mp->ocupado = true;
	strcpy(mp->contenido, pedido->contenido);

	/* actualizar la TLB -> se hace fuera del switch */


	return mp;
}

int32_t getRandomFrameVacio(){
	bool frameVacio(t_MP* entrada){
		return entrada->ocupado==false;
	}
	t_MP* mp = list_find(memoriaPrincipal, (void*)frameVacio);
	return mp->marco;
}

void manejarMemoriaPrincipal(t_MP* entradaMP, sock_t* cpuSocket){
	if(entradaMP!=NULL){
		t_LecturaSwap* pedido = malloc(sizeof(t_LecturaSwap));
		pedido->longitud = string_length(entradaMP->contenido)+1;
		pedido->contenido = malloc(pedido->longitud);
		pedido->encontro=1;
		strcpy(pedido->contenido, entradaMP->contenido);

		enviarContenidoPagina(cpuSocket,pedido);
		free(pedido->contenido);
		free(pedido);
	}
}

int32_t calcularCantPaginasEnMP(int32_t idmProc){
	bool porPIDyPresente(t_TP* entrada){
			return entrada->idProc==idmProc && entrada->present==1;
	}
	int32_t cantidad = list_count_satisfying(tablasDePaginas, (void*) porPIDyPresente);
	return cantidad;
}

t_TLB* buscarEnTLB(int32_t idmProc, int32_t nroPagina){
	bool porNroPaginaYProceso(t_TLB* entrada){
		return entrada->idProc ==idmProc && entrada->pagina==nroPagina;
	}
	t_TLB* entradaTLB = list_find(TLB,(void*)porNroPaginaYProceso);
	return entradaTLB;
}

t_MP* buscarEnMemoriaPrincipal(int32_t marco){
	bool porMarco(t_MP* entrada){
		return entrada->marco==marco;
	}
	t_MP* hit = list_find(memoriaPrincipal, (void*) porMarco);
	return hit;
}

int32_t buscarMarcoEnTablaDePaginas(int32_t idmProc, int32_t nroPagina){
	bool porPIDyPag(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->nroPag==nroPagina;
	}
	t_TP* tablaPagina = list_find(tablasDePaginas, (void*) porPIDyPag);
	if(tablaPagina==NULL){
		printf("No se encontró la página en la tabla de este proceso. RESCATE, ME ESTÁS PIDIENDO UNA PÁGINA QUE NO ES TUYA.");
		return -1;
	}else{
		if(tablaPagina->present==1){
			return tablaPagina->frame;
		} else {
			return swap_in; /* pedir a Swap */
		}
	}
}
