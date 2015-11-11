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
		tabla->usedTime = REINIT;
		tabla->loadedTime = REINIT;
		tabla->frame = marco_inhabilitado;
		tabla->nroPag = i;
		list_add(tablasDePaginas,tabla);
	}
	return pedido_exitoso;
}

/* unico reemplazo -> FIFO */
t_TLB* actualizarTLB(int32_t idmProc, int32_t nroPagina, int32_t marco){
	if(list_size(TLB)>=configuracion->entradas_tlb){
		list_remove_and_destroy_element(TLB,0,(void*) TLBDestroyer);
	}
	t_TLB* nuevaEntrada = malloc(sizeof(t_TLB));
	nuevaEntrada->idProc = idmProc;
	nuevaEntrada->pagina = nroPagina;
	nuevaEntrada->marco = marco;

	list_add(TLB, nuevaEntrada);
	return nuevaEntrada;
}

void eliminarTablaDePaginas(int32_t idmProc){

	bool porPID(t_TP* entrada){
		return entrada->idProc==idmProc;
	}
	list_remove_and_destroy_by_condition(tablasDePaginas, (void*) porPID, (void*)procesoDestroyer);
}

void vaciarMarcosOcupados(int32_t idmProc){
	t_list* paginasPresentes = getTablaDePaginasPresentes(idmProc);

	void vaciarContenidoMP(t_TP* entrada){
		if(entrada->present){
			bool porMarco(t_MP* mp){
				return mp->marco==entrada->frame;
			}
			t_MP* entradaADesocupar = list_find(memoriaPrincipal, (void*) porMarco);
			entradaADesocupar->ocupado=false;
		}
	}
	list_iterate(paginasPresentes, (void*) vaciarContenidoMP);
}

int32_t swapIN(sock_t* swapSocket, sock_t* cpuSocket, int32_t idmProc, int32_t nroPagina, int32_t codigo){

	int32_t marcoAReemplazar;
	int32_t cantMarcosOtorgados = calcularCantPaginasEnMP(idmProc);
	if(cantMarcosOtorgados>=configuracion->maximo_marcos_por_proceso) {
		/* Swap Out con reemplazo local */
		marcoAReemplazar = reemplazarMP(idmProc, configuracion->algoritmo_reemplazo);
	} else {
		marcoAReemplazar = getRandomFrameVacio();
	}

	if(marcoAReemplazar==marcos_insuficientes || marcoAReemplazar==marcos_no_libres){

		t_LecturaSwap* pedido = malloc(sizeof(t_LecturaSwap));
		pedido->encontro=pedido_error;
		enviarEnteros(cpuSocket,pedido->encontro);
//		free(pedido->contenido);
		free(pedido);

		if(marcoAReemplazar==marcos_insuficientes){
			log_info(MemoriaLog, " - *Proceso Abortado* - Razón: Falta de marcos para reemplazo local\n");
			abortarProceso(idmProc);
			return marcos_insuficientes;
		}else{
			log_info(MemoriaLog, " - *Proceso Abortado* - Razón: Falta de marcos disponibles\n");
			abortarProceso(idmProc);
			return marcos_no_libres;
		}
	}

	t_TP* entradaARemoverDeMP = buscarEnTablaDePaginasByMarco(marcoAReemplazar);
	if(entradaARemoverDeMP!=NULL && entradaARemoverDeMP->modified==true){
		if(escribirEnSwap(entradaARemoverDeMP, swapSocket)){
			entradaARemoverDeMP->modified=false;
		}
	}

	t_LecturaSwap* pedido = pedirPagina(swapSocket,idmProc, nroPagina);
		if(pedido==NULL || pedido->encontro==false) {
			enviarEnteros(cpuSocket, pedido_error);
			return -1;
		}
		log_info(MemoriaLog, " - *Acceso a SWAP*  PID: %d", idmProc);

	t_MP* mp = actualizarMP(idmProc, nroPagina, marcoAReemplazar, pedido);

	if(codigo==codigo_leer){

		//DEBERIA BUSCAR EN MEMORIA PRINCIPAL ACTUALIZADA...
		retardo(configuracion->retardo_memoria, memoria_principal, idmProc, nroPagina, marcoAReemplazar);

		enviarContenidoPagina(cpuSocket, pedido);
	}
	free(pedido->contenido);
	free(pedido);

	return mp->marco;
}

bool escribirEnSwap(t_TP* entradaARemoverDeMP, sock_t* swapSocket){

	t_MP* mp = buscarEnMemoriaPrincipal(entradaARemoverDeMP->frame);

	enviarEnteros(swapSocket, codigo_escribir);
	enviarEnteros(swapSocket, entradaARemoverDeMP->idProc);
	enviarEnteros(swapSocket, entradaARemoverDeMP->nroPag);
	enviarStrings(swapSocket, mp->contenido, configuracion->tamanio_marco);

	int32_t confirmacionSwap;
	int32_t recibidoConfirmacion = recv(swapSocket->fd, &confirmacionSwap, sizeof(int32_t), 0);
	if(recibidoConfirmacion<=0){
		log_error(MemoriaLog,RED "No se recibió la confirmación de Swap\n"RESET);
		return false;
	}
	if(confirmacionSwap==pedido_error){
		log_error(MemoriaLog,RED "No se pudo guardar la página en la partición\n"RESET);
		return false;
	} else {
		log_info(MemoriaLog, "Se escribió la página en la partición Swap\n");
		return true;
	}
}


t_MP* actualizarMP(int32_t idmProc, int32_t nroPagina, int32_t marcoAReemplazar, t_LecturaSwap* pedido){

	/* saco la página de MP y le inhabilito el marco*/
	bool porMarco(t_TP* entrada){
			return entrada->frame==marcoAReemplazar;
	}
	t_TP* paginaSwappedOut = list_find(tablasDePaginas, (void*) porMarco);
	if(paginaSwappedOut!=NULL){
		paginaSwappedOut->present = false;
		paginaSwappedOut->frame = marco_inhabilitado;
	}

	/* llevo la página a MP y le asigno el marco otorgado*/
	bool porPIDyPag(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->nroPag==nroPagina;
	}
	t_TP* paginaSwappedIn = list_find(tablasDePaginas, (void*) porPIDyPag);
	paginaSwappedIn->present=true;
	paginaSwappedIn->frame = marcoAReemplazar;


	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, FIFO)){
		paginaSwappedIn->loadedTime = setLoadedTimeForProc(idmProc);
	}

	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, LRU)){
		if(paginaSwappedOut!=NULL){
			paginaSwappedOut->usedTime = REINIT;
		}
		paginaSwappedIn->usedTime = 0;
	}

	/* por las dudas le pongo el ocupado */
	t_MP* mp = buscarEnMemoriaPrincipal(paginaSwappedIn->frame);
	mp->ocupado = true;
	strcpy(mp->contenido, pedido->contenido);

	/* actualizar la TLB -> se hace fuera del switch */

	return mp;
}

void eliminarDeTLBPorMarco(int32_t marco){
	bool porMarco(t_TLB* entrada){
			return entrada->marco==marco;
	}
	list_remove_and_destroy_by_condition(TLB, (void*) porMarco, (void*) TLBDestroyer);
}

void eliminarDeTLBPorPID(int32_t idmProc){
	bool porPID(t_TLB* entrada){
		return entrada->idProc==idmProc;
	}
	list_remove_and_destroy_by_condition(TLB, (void*) porPID, (void*) TLBDestroyer);
}

t_list* getTablaDePaginasPresentes(int32_t idmProc){
	bool porPIDYPresent(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->present==true;
	}
	t_list* tablaDelProceso = list_filter(tablasDePaginas, (void*) porPIDYPresent);
	return tablaDelProceso;
}


int32_t setLoadedTimeForProc(int32_t idmProc){

	t_list* tablaDelProceso = getTablaDePaginasPresentes(idmProc);

	if(list_size(tablaDelProceso)==0){
		return 0;
	} else{
		int32_t minLoadedTime = getMinLoadedTime(tablaDelProceso);
		return minLoadedTime+1;
	}
}

int32_t getMinLoadedTime(t_list* tablaDelProceso){
	int32_t min=RAND_MAX;
	void minimo(t_TP* entrada){
		if(entrada->loadedTime<min){
			min=entrada->loadedTime;
		}
	}
	list_iterate(tablaDelProceso, (void*) minimo);
	return min;
}

int32_t getMaxUsedTime(t_list* tablaDelProceso){
	int32_t max=REINIT;
	void maximo(t_TP* entrada){
		if(entrada->usedTime>max){
			max=entrada->usedTime;
		}
	}
	list_iterate(tablaDelProceso, (void*) maximo);
	return max;
}

int32_t getRandomFrameVacio(){
	bool frameVacio(t_MP* entrada){
		return entrada->ocupado==false;
	}
	t_MP* mp = list_find(memoriaPrincipal, (void*)frameVacio);
	if(mp!=NULL){
		return mp->marco;
	} else{
		return marcos_no_libres;
	}

}


void manejarMemoriaPrincipalEscritura(t_MP* entradaMP, sock_t* cpuSocket, char* contenidoAEscribir, int32_t idmProc, int32_t nroPagina){
	if(entradaMP!=NULL){
		strcpy(entradaMP->contenido, contenidoAEscribir);

		llenarDeNulos(entradaMP->contenido,configuracion->tamanio_marco,string_length(contenidoAEscribir));

		log_info(MemoriaLog, "Se escribió en la página %d del proceso %d  el contenido: %s\n", nroPagina, idmProc, entradaMP->contenido);

		printf ("%s\n", contenidoAEscribir);

		t_TP* tablaPag = buscarEntradaEnTablaDePaginas(idmProc,nroPagina);
		if(tablaPag!=NULL){
			tablaPag->modified=true;
			enviarEnteros(cpuSocket,pedido_exitoso);
		} else{
			log_error(MemoriaLog,RED "No se encontró la página en la tabla de páginas\n"RESET);
			enviarEnteros(cpuSocket,pedido_error);
		}
	}
	else {
		log_error(MemoriaLog, "Entrada de memoria Nula\n");
		printf("Error!\n");
			}
}

void manejarMemoriaPrincipalLectura(t_MP* entradaMP, sock_t* cpuSocket){
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

int32_t reemplazarMP(int32_t idmProc, char* algoritmo_reemplazo){

	bool porPIDYPresent(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->present==true;
	}
	t_list* tablaDelProceso = list_filter(tablasDePaginas, (void*) porPIDYPresent);

	if(string_equals_ignore_case(algoritmo_reemplazo, FIFO)){
		return reemplazarFIFO(tablaDelProceso);
	}

	if(string_equals_ignore_case(algoritmo_reemplazo, LRU)){
		return reemplazarLRU(tablaDelProceso);
	}

	/*no estaría funcionando aún */
	if(string_equals_ignore_case(algoritmo_reemplazo, CLOCKM)){
		return reemplazarCLOCKM(tablaDelProceso);
	}

	return marcos_insuficientes;
}

int32_t reemplazarFIFO(t_list* tablaDelProceso){
	if(list_size(tablaDelProceso)==0){
		return marcos_insuficientes;
	} else{
		int32_t minLoaded = getMinLoadedTime(tablaDelProceso);

		bool porMinLoaded(t_TP* entrada){
			return entrada->loadedTime == minLoaded;
		}
		t_TP* aReemplazar = list_find(tablaDelProceso,(void*) porMinLoaded);
		return aReemplazar->frame;
	}
}

int32_t reemplazarCLOCKM(t_list* tablaDelProceso){
	return marcos_insuficientes;
}

int32_t reemplazarLRU(t_list* tablaDelProceso){

	if(list_size(tablaDelProceso)==0){
			return marcos_insuficientes;
	} else{
		int32_t maxUsed = getMaxUsedTime(tablaDelProceso);

		bool porMaxUsed(t_TP* entrada){
			return entrada->usedTime == maxUsed;
		}
		t_TP* aReemplazar = list_find(tablaDelProceso,(void*) porMaxUsed);

		void pasarTiempo(t_TP* pagina){
			if(pagina->usedTime!=REINIT){
				(pagina->usedTime)++;
			}
		}
		list_iterate(tablaDelProceso, (void*) pasarTiempo);
		return aReemplazar->frame;
	}
}

void abortarProceso(int32_t idmProc){
	vaciarMarcosOcupados(idmProc);
	eliminarTablaDePaginas(idmProc);

	if(configuracion->tlb_habilitada){
		eliminarDeTLBPorPID(idmProc);
	}
}

int32_t calcularCantPaginasEnMP(int32_t idmProc){
	bool porPIDyPresente(t_TP* entrada){
			return entrada->idProc==idmProc && entrada->present==1;
	}
	int32_t cantidad = list_count_satisfying(tablasDePaginas, (void*) porPIDyPresente);
	return cantidad;
}

void retardo(int32_t cantidad, int32_t donde, int32_t idmProc, int32_t pag, int32_t marco){
	if(donde==tabla_paginas){
		printf("Buscando en la tabla de páginas... \n");
	}
	if(donde==memoria_principal){
		log_info(MemoriaLog, "- *Acceso a Memoria* PID: %d, Nro. Página: %d, Nro. Marco: %d \n", idmProc, pag, marco);
	}
	sleep(cantidad);
}

void llenarDeNulos(char* destino, int32_t total, int32_t desfazaje){
	int32_t index;
	for(index=0; index<(total-desfazaje); index++){
		memcpy(destino+index+desfazaje, "\0", sizeof(char));
	}
}
