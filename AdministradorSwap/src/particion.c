#include "administradorSwap.h"

void crearParticion()
{
	char instruccion[1000]={0};
	sprintf(instruccion, "dd if=/dev/zero of=%s bs=%d count=%d", configuracion->nombre_swap,configuracion->tamano_pagina,configuracion->cantidad_paginas);
	system(instruccion);
}

void inicializarParticion()
{
	crearParticion();
	espacioLibre = list_create();
	espacioOcupado = list_create();
	t_nodoLibre* nodoLibre = malloc(sizeof(t_nodoLibre));
	nodoLibre->comienzo = 0;
	nodoLibre->paginas = configuracion->cantidad_paginas;
	list_add(espacioLibre,nodoLibre);
}

void eliminarParticion()
{
	if (remove(configuracion->nombre_swap) == 0){
		printf("Elimino correctamente la particion \n");
	}
	else{
		printf("No se elimino correctamente la particion \n");
	}
}

uint32_t contarPaginasLibres()
{
	uint32_t total = 0;
	uint32_t sizeLista = list_size(espacioLibre);
	uint32_t indice;
	t_nodoLibre* nodo;
	for(indice = 0;indice < sizeLista;indice++)
	{
		nodo = list_get(espacioLibre,indice);
		total = total + nodo->paginas;
	}

	return total;
}

//retorna 0 si es falso y 1 si es verdadero
short hayEspacio(uint32_t paginas)
{
	uint32_t paginasLibres = contarPaginasLibres();
	return paginasLibres >= paginas;
}

bool hayEspacioSecuencial(uint32_t paginas)
{
	paginasProceso = paginas;
	bool resultado = list_any_satisfy(espacioLibre,validarEspacioLibre);
	return resultado;
}

bool validarEspacioLibre(void* nodo)
{
	t_nodoLibre* nodoLibre = nodo;
	bool resultado = nodoLibre->paginas >= paginasProceso;
	return resultado;
}

//Se ejecuta funcion despues de validar
void ocuparEspacio(uint32_t PID,uint32_t paginasAOcupar)
{
	t_nodoLibre* nodoLibre;
	t_nodoOcupado* nodoOcupado = malloc(sizeof(t_nodoOcupado));
	nodoOcupado->paginas = paginasAOcupar;
	nodoOcupado->PID = PID;
	paginasProceso = paginasAOcupar;
	nodoLibre = list_find(espacioLibre, validarEspacioLibre);
	nodoOcupado->comienzo = nodoLibre->comienzo;
	if(nodoLibre->paginas > paginasAOcupar)
	{
		nodoLibre->comienzo = nodoLibre->comienzo + paginasAOcupar;
		nodoLibre->paginas = nodoLibre->paginas - paginasAOcupar;
	}
	else
	{
		//Cantidad de paginas a ocupar es igual a cantidad libre
		ubicacionNodo = nodoLibre->comienzo;
		list_remove_by_condition(espacioLibre, validarUbicacionLibre);
	}
	list_add(espacioOcupado,nodoOcupado);
}

bool validarUbicacionLibre(void* nodo)
{
	t_nodoLibre* nodoLibre = nodo;
	return nodoLibre->comienzo == ubicacionNodo;
}
