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

void eliminarOrdenMarcos(int32_t idmProc){
	bool porPID(t_Marcos* entrada){
		return entrada->idProc == idmProc;
	}
	int32_t veces = list_count_satisfying(ordenMarcos, (void*)porPID);
	int32_t iterador;
	for(iterador=0; iterador<veces; iterador++){
		list_remove_and_destroy_by_condition(ordenMarcos,(void*)porPID, (void*) marcosDestroyer);
	}
}

void eliminarTablaDePaginas(int32_t idmProc){

	bool porPID(t_TP* entrada){
		return entrada->idProc==idmProc;
	}
	int32_t veces = list_count_satisfying(tablasDePaginas, (void*) porPID);
	int32_t it = 0;
	for(it=0; it<veces;it++){
		list_remove_and_destroy_by_condition(tablasDePaginas, (void*) porPID, (void*)procesoDestroyer);
	}
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

		/*Si no hay nuevo marco pero puedo desalojar de los que tengo*/
		if(marcoAReemplazar == marcos_no_libres && cantMarcosOtorgados>0){
			marcoAReemplazar = reemplazarMP(idmProc, configuracion->algoritmo_reemplazo);
			int32_t marco = doSwap(idmProc, nroPagina, marcoAReemplazar, codigo, swapSocket, cpuSocket);
			return marco;

		} else {  /* no tengo para desalojar */

			if(marcoAReemplazar==marcos_insuficientes || marcoAReemplazar==marcos_no_libres){
				t_LecturaSwap* pedido = malloc(sizeof(t_LecturaSwap));
				pedido->encontro=pedido_error;
				enviarEnteros(cpuSocket,pedido->encontro);
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
		}
	}
	int32_t marco = doSwap(idmProc, nroPagina, marcoAReemplazar, codigo, swapSocket, cpuSocket);
	return marco;
}

int32_t doSwap(int32_t idmProc, int32_t nroPagina, int32_t marcoAReemplazar, int32_t codigo, sock_t* swapSocket, sock_t* cpuSocket){
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
	printf("Busca\n");
	printf("marco a limpiar %d\n", entradaARemoverDeMP->frame);
	printf("contenido a sacar %s\n", mp->contenido);

	printf("socket swap: %d\n", clientSocketSwap->fd);
	enviarEnteros(clientSocketSwap, codigo_escribir);
	printf("se eenvio el codigo \n");
	enviarEnteros(clientSocketSwap, entradaARemoverDeMP->idProc);
	printf("se eenvio el id de proc \n");
	enviarEnteros(clientSocketSwap, entradaARemoverDeMP->nroPag);
	printf("se eenvio el nro de pag \n");
	enviarStrings(clientSocketSwap, mp->contenido, configuracion->tamanio_marco);
	printf("se eenvio el contenido \n");

	printf("Envia todo \n");
	int32_t confirmacionSwap;
	int32_t recibidoConfirmacion = recv(clientSocketSwap->fd, &confirmacionSwap, sizeof(int32_t), 0);
	printf("recibido confirmacion %d\n", recibidoConfirmacion);
	if(recibidoConfirmacion<=0){
		log_error(MemoriaLog,RED "No se recibió la confirmación de Swap\n"RESET);
		return false;
	}

	printf("recibe confirmacion \n");
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
		paginaSwappedIn->loadedTime = 0;
	}

	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, LRU)){
		if(paginaSwappedOut!=NULL){
			paginaSwappedOut->usedTime = REINIT;
		}
		paginaSwappedIn->usedTime = 0;
	}

	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, CLOCKM)){
		printf("llega a querer actualizar el clock \n");
		bool porPID(t_Marcos* entrada){
			return entrada->idProc == idmProc;
		}
		t_Marcos* marcoMP = list_find(ordenMarcos, (void*)porPID);
		if(marcoMP==NULL){
			crearElementoOrdenMarcos(idmProc, nroPagina, marcoAReemplazar, 0, true);

		} else if(list_size(marcoMP->marcos) < configuracion->maximo_marcos_por_proceso){
			int32_t orden = list_size(marcoMP->marcos);
			bool buscarPuntero(t_Orden* entrada){
				return entrada->puntero == true;
			}
			t_Orden* puntero = list_find(marcoMP->marcos, (void*) buscarPuntero);
			bool punt;
			if(puntero==NULL){
				punt=true;
			} else {
				punt=false;
			}

			crearElementoOrden(nroPagina, marcoAReemplazar, orden, punt, marcoMP->marcos);
		} else{
			bool porMarco(t_Orden* entrada){
				return entrada->marco == marcoAReemplazar;
			}
			t_Orden* puntero = list_find(marcoMP->marcos, (void*) porMarco);
			puntero->nroPag = nroPagina;
		}

		if(paginaSwappedOut!=NULL){
			paginaSwappedOut->accessed = false;
		}
		paginaSwappedIn->accessed = true;
	}

	/* por las dudas le pongo el ocupado */
	t_MP* mp = buscarEnMemoriaPrincipal(paginaSwappedIn->frame);
	mp->ocupado = true;
	strcpy(mp->contenido, pedido->contenido);

	/* actualizar la TLB -> se hace fuera del switch */

	return mp;
}
void avanzarTiempo(int32_t idmProc, int32_t nroPag){
	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, LRU)){
		avanzarTiempoLRU(idmProc, nroPag);
	}

	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, FIFO)){
		avanzarTiempoFIFO(idmProc);
	}
}

void avanzarTiempoFIFO(int32_t idmProc){
	t_list* tablaDelProceso = getTablaDePaginasPresentes(idmProc);
	if(list_size(tablaDelProceso)!=0){
		void avanzarTiempoF(t_TP* e){
			(e->loadedTime)++;
		}
		list_iterate(tablaDelProceso, (void*)avanzarTiempoF);
	}
}
void avanzarTiempoLRU(int32_t idmProc, int32_t nroPag){
	t_list* tablaDelProceso = getTablaDePaginasPresentes(idmProc);

	if(list_size(tablaDelProceso)!=0){
		void avanzarTiempoL(t_TP* e){
			if(e->nroPag==nroPag){
				e->usedTime=REINIT;
			}else{
				(e->usedTime)++;
			}
		}
		list_iterate(tablaDelProceso, (void*)avanzarTiempoL);
	}
}

t_Marcos* crearElementoOrdenMarcos(int32_t idmProc, int32_t nroPagina, int32_t marcoAReemplazar, int32_t ordenAColocar, bool puntero){
	t_Marcos* marco = malloc(sizeof(t_Marcos));
	marco->idProc = idmProc;
	marco->marcos = list_create();
	crearElementoOrden(nroPagina,marcoAReemplazar, ordenAColocar, puntero, marco->marcos);
	list_add(ordenMarcos, marco);
	return marco;
}

t_Orden* crearElementoOrden(int32_t nroPagina, int32_t marcoAReemplazar, int32_t ordenAColocar, bool puntero, t_list* marcos){
	t_Orden* orden = malloc(sizeof(t_Orden));
	orden->marco = marcoAReemplazar;
	orden->orden = ordenAColocar;
	orden->nroPag = nroPagina;
	orden->puntero = puntero;
	list_add(marcos, orden);
	return orden;
}

void eliminarDeTLBPorMarco(int32_t marco){
	bool porMarco(t_TLB* entrada){
			return entrada->marco==marco;
	}
	int32_t veces = list_count_satisfying(TLB, (void*) porMarco);
	int32_t it = 0;
	for(it=0; it<veces;it++){
		list_remove_and_destroy_by_condition(TLB, (void*) porMarco, (void*) TLBDestroyer);
	}
}

void eliminarDeTLBPorPID(int32_t idmProc){
	bool porPID(t_TLB* entrada){
		return entrada->idProc==idmProc;
	}
	int32_t veces = list_count_satisfying(TLB, (void*) porPID);
	int32_t it = 0;
	for(it=0; it<veces;it++){
		list_remove_and_destroy_by_condition(TLB, (void*) porPID, (void*) TLBDestroyer);
	}
}

t_list* getTablaDePaginasPresentes(int32_t idmProc){
	bool porPIDYPresent(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->present==true;
	}
	t_list* tablaDelProceso = list_filter(tablasDePaginas, (void*) porPIDYPresent);
	return tablaDelProceso;
}

int32_t getMaxLoadedTime(t_list* tablaDelProceso){
	int32_t max=REINIT;
	void maximo(t_TP* entrada){
		if(entrada->loadedTime>max){
			max=entrada->loadedTime;
		}
	}
	list_iterate(tablaDelProceso, (void*) maximo);
	return max;
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

		if(string_length(contenidoAEscribir)>configuracion->tamanio_marco){
			log_info(MemoriaLog, "El contenido a escribir excede el tamaño del marco. Se recortará el contenido a la máxima extensión posible\n");
			contenidoAEscribir[(configuracion->tamanio_marco)-1] = '\0';
		}
		strcpy(entradaMP->contenido, contenidoAEscribir);
		llenarDeNulos(entradaMP->contenido,configuracion->tamanio_marco,string_length(contenidoAEscribir));

		log_info(MemoriaLog, "Se escribió en la página %d del proceso %d  el contenido: %s\n", nroPagina, idmProc, entradaMP->contenido);

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
		//TODO
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

	if(string_equals_ignore_case(algoritmo_reemplazo, CLOCKM)){
		return reemplazarCLOCKM(tablaDelProceso, idmProc);
	}

	return marcos_insuficientes;
}

int32_t reemplazarFIFO(t_list* tablaDelProceso){
	if(list_size(tablaDelProceso)==0){
		return marcos_insuficientes;
	} else{

		int32_t maxLoaded = getMaxLoadedTime(tablaDelProceso);
		bool porMaxLoaded(t_TP* entrada){
			return entrada->loadedTime == maxLoaded;
		}
		t_TP* aReemplazar = list_find(tablaDelProceso,(void*) porMaxLoaded);
		return aReemplazar->frame;
	}
}

int32_t reemplazarCLOCKM(t_list* tablaDelProceso, int32_t idmProc){

	if(list_size(tablaDelProceso)==0){
		return marcos_insuficientes;
	} else {
		bool porPID(t_Marcos* entrada){
			return entrada->idProc == idmProc;
		}
		t_Marcos* proc = list_find(ordenMarcos, (void*)porPID);

		ordenarPorCargaMarcos(proc->marcos);

		void printearr(t_Orden* ord){
			printf("marco: %d, orden %d, puntero %d \n", ord->marco, ord->orden, ord->puntero);
		}
		list_iterate(proc->marcos, (void*) printearr);

		int32_t marcoVictima = marco_victima;

		bool buscar00(t_Orden* entrada){
			bool porPIDyPag(t_TP* entry){
				return entry->idProc == idmProc && entry->nroPag == entrada->nroPag;
			}
			t_TP* pagina = list_find(tablaDelProceso, (void*)porPIDyPag);
			if(!pagina->accessed && !pagina->modified){
				marcoVictima = pagina->frame;
				pagina->accessed = false;
				return true;

			}else{
				if(pagina->accessed){
					pagina->accessed = false;
				}
				return false;
			}
		}
		t_Orden* victima = list_find(proc->marcos, (void*) buscar00);

		if(victima!=NULL && marcoVictima!=marco_victima){
			adelantarPuntero(proc->marcos);
			return marcoVictima;
		}

		bool buscar01(t_Orden* entrada){
			bool porPIDyPag(t_TP* entry){
				return entry->idProc == idmProc && entry->nroPag == entrada->nroPag;
			}
			t_TP* pagina = list_find(tablaDelProceso, (void*)porPIDyPag);
			return !pagina->accessed && pagina->modified;
		}
		victima = list_find(proc->marcos, (void*) buscar01);

		if(victima!=NULL){
			adelantarPuntero(proc->marcos);
			return victima->marco;
		}

		bool buscar10(t_Orden* entrada){
			bool porPIDyPag(t_TP* entry){
				return entry->idProc == idmProc && entry->nroPag == entrada->nroPag;
			}
			t_TP* pagina = list_find(tablaDelProceso, (void*)porPIDyPag);
			return pagina->accessed && !pagina->modified;
		}
		victima = list_find(proc->marcos, (void*) buscar10);

		if(victima!=NULL){
			adelantarPuntero(proc->marcos);
			return victima->marco;
		}

		bool buscar11(t_Orden* entrada){
			bool porPIDyPag(t_TP* entry){
				return entry->idProc == idmProc && entry->nroPag == entrada->nroPag;
			}
			t_TP* pagina = list_find(tablaDelProceso, (void*)porPIDyPag);
			return pagina->accessed && pagina->modified;
		}
		victima = list_find(proc->marcos, (void*) buscar11);

		adelantarPuntero(proc->marcos);
		return victima->marco;
	}

}

void adelantarPuntero(t_list* marcos){
	t_Orden* ord0 = list_get(marcos,0);
	ord0->puntero = 0;

	t_Orden* ord1 = list_get(marcos,1);
	ord1->puntero = 1;
}

/* no se usa */
void buscarPar(t_TP* pagina, bool accessed, bool modified, int32_t marcoVictima){

	bool condition ;

	if(!accessed && !modified){
		condition = !pagina->accessed && !pagina->modified;
	}

	if(accessed && !modified){
		condition = pagina->accessed && !pagina->modified;
	}

	if(!accessed && modified){
		condition = !pagina->accessed && pagina->modified;
	}

	if(accessed && modified){
		condition = pagina->accessed && pagina->modified;
	}

	if(condition){
		marcoVictima = pagina->frame;
	}
}

void ordenarPorCargaMarcos(t_list* marcos){

	bool anterior(t_Orden* ord1, t_Orden* ord2){
		return ord1->orden < ord2->orden;
	}
	list_sort(marcos, (void*) anterior);

	int32_t i;
	for(i=0; i<list_size(marcos); i++ ){
		t_Orden* orden = list_get(marcos,i);
		if(orden->puntero){
			t_list* ordenados = list_take_and_remove(marcos, i);
			list_add_all(marcos,ordenados);
		}
	}
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

void retardo(double cantidad, int32_t donde, int32_t idmProc, int32_t pag, int32_t marco){
	if(donde==tabla_paginas){
		printf("Buscando en la tabla de páginas... \n");
	}
	if(donde==memoria_principal){
		log_info(MemoriaLog, "- *Acceso a Memoria* PID: %d, Nro. Página: %d, Nro. Marco: %d \n", idmProc, pag, marco);
	}
	usleep(cantidad*1000000);
}

void llenarDeNulos(char* destino, int32_t total, int32_t desfazaje){
	int32_t index;
	for(index=0; index<(total-desfazaje); index++){
		memcpy(destino+index+desfazaje, "\0", sizeof(char));
	}
}
