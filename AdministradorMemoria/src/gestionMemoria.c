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

void swapIN(sock_t* swapSocket, sock_t* cpuSocket, int32_t idmProc, int32_t nroPagina){
	t_LecturaSwap* pedido = pedirPagina(swapSocket,idmProc, nroPagina);
	if(pedido==NULL || pedido->encontro==false)
	{
		enviarEnteros(cpuSocket, pedido_error);
		return;
	}
	log_info(MemoriaLog, " - *Acceso a SWAP*  PID: %d", idmProc);

	enviarContenidoPagina(cpuSocket, pedido);

	//TODO metodo para buscar en tabla de paginas que devuelva toda la entrada
	bool porPIDyPag(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->nroPag==nroPagina;
	}
	t_TP* paginaEncontrada = list_find(tablasDePaginas, (void*) porPIDyPag);
	paginaEncontrada->present=1;

	/* falta actualizar memoria principal con frame/pagina y copiar contenido */
	//TODO pasar a funcion actualizarMP();
	t_MP* mp = buscarEnMemoriaPrincipal(paginaEncontrada->frame);
	mp->ocupado = 1;
	strcpy(mp->contenido, pedido->contenido);


	free(pedido->contenido);
	free(pedido);
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

//int32_t getPagina(t_list* tablasDePaginas, int32_t idMProc)
//{
//	int32_t pagina =  rand() % 1000;
//	bool porProcesoYNroPagina(void * entrada)
//	{
//		t_TP* tabla=(t_TP*) entrada;
//		return tabla->nroPag==pagina && tabla->idProc==idMProc;
//	}
//	t_TP* encontrado = list_find(tablasDePaginas,porProcesoYNroPagina);
//	while(encontrado!=NULL)
//	{
//		pagina =  rand() % 1000;
//		bool porProcesoYNroPagina(void * entrada)
//		{
//			t_TP* tabla=(t_TP*) entrada;
//			return tabla->nroPag==pagina && tabla->idProc==idMProc;
//		}
//		encontrado = list_find(tablasDePaginas,porProcesoYNroPagina);
//	}
//	return pagina;
//}

