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
	mappear_archivo();
}

void eliminarParticion()
{
	if (remove(configuracion->nombre_swap) == 0){
		printf("Elimino correctamente la particion \n");
	}
	else{
		printf("No se elimino correctamente la particion \n");
	}
	munmap(archivoMapeado->memoria,archivoMapeado->tamanio);
	free(archivoMapeado->memoria);
	free(archivoMapeado);
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
	paginasCondicion = paginas;
	bool resultado = list_any_satisfy(espacioLibre,validarEspacioLibre);
	return resultado;
}

bool validarEspacioLibre(void* nodo)
{
	t_nodoLibre* nodoLibre = nodo;
	bool resultado = nodoLibre->paginas >= paginasCondicion;
	return resultado;
}

//Se ejecuta funcion despues de validar
void ocuparEspacio(uint32_t PID,uint32_t paginasAOcupar)
{
	t_nodoLibre* nodoLibre;
	t_nodoOcupado* nodoOcupado = malloc(sizeof(t_nodoOcupado));
	nodoOcupado->paginas = paginasAOcupar;
	nodoOcupado->PID = PID;
	paginasCondicion = paginasAOcupar;
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
		ubicacionCondicion = nodoLibre->comienzo;
		nodoLibre = list_remove_by_condition(espacioLibre, validarUbicacionLibre);
		free(nodoLibre);
	}
	list_add(espacioOcupado,nodoOcupado);
}

void liberarEspacio(uint32_t PID)
{	t_nodoOcupado* nodoOcupado;
	pidCondicion = PID;
	nodoOcupado = list_remove_by_condition(espacioOcupado,validarMismoPid);
    t_nodoLibre* nodoLibre = malloc(sizeof(t_nodoLibre));
    nodoLibre->comienzo = nodoOcupado->comienzo;
    nodoLibre->paginas = nodoOcupado->paginas;
    list_add(espacioLibre,nodoLibre);
    free(nodoOcupado);
}

bool validarMismoPid(void* nodo)
{
	t_nodoOcupado* nodoOcupado = nodo;
	return nodoOcupado->PID == pidCondicion;
}

bool validarUbicacionLibre(void* nodo)
{
	t_nodoLibre* nodoLibre = nodo;
	return nodoLibre->comienzo == ubicacionCondicion;
}

void mappear_archivo()
{
	archivoMapeado = malloc(sizeof(t_archivoSwap));
	archivoMapeado->memoria = malloc(configuracion->tamano_pagina * configuracion->cantidad_paginas);
	int tamanio = 0;
	FILE *archivo = fopen(configuracion->nombre_swap, "r");
	if (archivo == NULL)
	{
		printf("No se pudo abrir el archivo \n");
	}
	else
	{
		fseek(archivo, 0L, SEEK_END);
		tamanio = ftell(archivo);
		fclose(archivo);
	}
	archivoMapeado->fd = open(configuracion->nombre_swap, O_RDWR, (mode_t) 0600);
	if(archivoMapeado->fd == -1){
		perror("open");
	}
	archivoMapeado->memoria = (char*) mmap(NULL, tamanio, PROT_WRITE | PROT_READ,MAP_SHARED, archivoMapeado->fd, 0);
	archivoMapeado->tamanio = tamanio;
}

char* buscarPagina(uint32_t PID, uint32_t pagina)
{
	char* paginaBuscada = malloc(sizeof(configuracion->tamano_pagina));
	pidCondicion = PID;
	t_nodoOcupado* nodo = list_find(espacioOcupado,validarMismoPid);
	uint32_t ubicacionPagina = nodo->comienzo + (pagina - 1);
	memcpy(paginaBuscada,archivoMapeado->memoria + ubicacionPagina*configuracion->tamano_pagina,configuracion->tamano_pagina);
	return paginaBuscada;
}

void escribirPagina(char* pagina,uint32_t PID,uint32_t ubicacion)
{
	pidCondicion = PID;
	t_nodoOcupado* nodo = list_find(espacioOcupado,validarMismoPid);
	uint32_t posicionEnArchivo = (nodo->comienzo + ubicacion)*configuracion->tamano_pagina;
	char* areaMemoriaAEscribir = archivoMapeado->memoria + posicionEnArchivo;
	memcpy(areaMemoriaAEscribir, pagina,configuracion->tamano_pagina);

}
