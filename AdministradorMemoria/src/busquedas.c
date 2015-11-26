/*
 * gestionMemoria.c
 *
 *  Created on: 23/10/2015
 *      Author: ElianaLS
 */

#include "administradorMemoria.h"


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
	//	log_error(MemoriaLog, RED "Se intentó acceder a una página que no corresponde al proceso\n" RESET);
		return -1;
	}else{
		if(tablaPagina->present==1){
			return tablaPagina->frame;
		} else {
			return swap_in; /* pedir a Swap */
		}
	}
}


t_TP* buscarEnTablaDePaginasByMarco(int32_t marco){
	bool porMarco(t_TP* entrada){
		return (entrada->frame==marco && entrada->present==true);  /* podría no ser necesario*/
	}
	t_TP* tablaPagina = list_find(tablasDePaginas, (void*) porMarco);

	return tablaPagina;
}

t_TP* buscarEntradaEnTablaDePaginas(int32_t idmProc, int32_t nroPagina){
	bool porPIDyPag(t_TP* entrada){
		return entrada->idProc==idmProc && entrada->nroPag==nroPagina;
	}
	t_TP* tablaPagina = list_find(tablasDePaginas, (void*) porPIDyPag);
	return tablaPagina;
}

t_Stats* buscarEstadisticaPorProceso(int32_t idmProc){
	bool porPID(t_Stats* entrada){
		return entrada->idProc==idmProc;
	}
	t_Stats* stat = list_find(estadisticas, (void*) porPID);
	return stat;
}
