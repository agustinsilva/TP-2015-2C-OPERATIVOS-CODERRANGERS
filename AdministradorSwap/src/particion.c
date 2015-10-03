#include "administradorSwap.h"

void crearParticion()
{
	char instruccion[1000]={0};

	//Genera archivo lleno de caracter \0
	sprintf(instruccion, "dd if=/dev/zero of=%s bs=%d count=%d", configuracion->nombre_swap,configuracion->tamano_pagina,configuracion->cantidad_paginas);

	//Descomentar para generar Archivo con A
//	int32_t tamanio = configuracion->cantidad_paginas*configuracion->tamano_pagina;
//	sprintf(instruccion, "< /dev/urandom tr -dc 'A' | head -c%d > %s",tamanio,configuracion->nombre_swap);

	system(instruccion);
	sprintf(instruccion, "clear");
	system(instruccion);
}

void inicializarParticion()
{
	crearParticion();
	espacioLibre = list_create();
	espacioOcupado = list_create();
	estadisticasProcesos = list_create();
	t_nodoLibre* nodoLibre = malloc(sizeof(t_nodoLibre));
	nodoLibre->comienzo = 0;
	nodoLibre->paginas = configuracion->cantidad_paginas;
	list_add(espacioLibre,nodoLibre);

	mappear_archivo();
}

void eliminarParticion()
{
	munmap(archivoMapeado->memoria,archivoMapeado->tamanio);
	close(archivoMapeado->fd);
	free(archivoMapeado);
	if (remove(configuracion->nombre_swap) == 0)
	{
		printf("Elimino correctamente la particion \n");
	}
	else
	{
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
bool hayEspacio(uint32_t paginas)
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

//Se ejecuta funcion despues de validar, devuelve posicion pagina inicial
uint32_t ocuparEspacio(uint32_t PID,uint32_t paginasAOcupar)
{
	uint32_t posicionInicial;
	t_nodoLibre* nodoLibre;
	t_nodoOcupado* nodoOcupado = malloc(sizeof(t_nodoOcupado));
	nodoOcupado->paginas = paginasAOcupar;
	nodoOcupado->PID = PID;
	paginasCondicion = paginasAOcupar;
	nodoLibre = list_find(espacioLibre, validarEspacioLibre);
	nodoOcupado->comienzo = nodoLibre->comienzo;
	posicionInicial = nodoOcupado->comienzo;
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
	return posicionInicial;
}

void liberarProceso(uint32_t PID)
{
	uint32_t byteInicial;
	uint32_t tamanio;
	t_nodoOcupado* nodoOcupado;
	pidCondicion = PID;
	nodoOcupado = list_remove_by_condition(espacioOcupado,validarMismoPid);
    byteInicial = nodoOcupado->comienzo * configuracion->tamano_pagina;
    tamanio = nodoOcupado->	paginas * configuracion->tamano_pagina;
    ubicacionCondicion = nodoOcupado->comienzo + nodoOcupado->paginas;
    t_nodoLibre* nodoLibre = list_find(espacioLibre,buscarNodoComienzo);
    if(nodoLibre == NULL)
    {
	nodoLibre = malloc(sizeof(t_nodoLibre));
    nodoLibre->comienzo = nodoOcupado->comienzo;
    nodoLibre->paginas = nodoOcupado->paginas;
    list_add(espacioLibre,nodoLibre);
    }
    else
    {
    	nodoLibre->comienzo = nodoOcupado->comienzo;
    	nodoLibre->paginas += nodoOcupado->paginas;
    }
    log_info(SwapLog,"Se libera proceso con PID %d, byte inicial %d y tamaño %d",PID,byteInicial,tamanio);
    free(nodoOcupado);
}

bool buscarNodoComienzo(void* nodo)
{
	t_nodoLibre* nodoLibre = nodo;
	return nodoLibre->comienzo == ubicacionCondicion;
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
	//archivoMapeado->memoria = malloc(configuracion->tamano_pagina * configuracion->cantidad_paginas);
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
	if(archivoMapeado->fd == -1)
	{
		perror("open");
	}
	archivoMapeado->memoria = (char*) mmap(NULL, tamanio, PROT_WRITE | PROT_READ,MAP_SHARED, archivoMapeado->fd, 0);
	archivoMapeado->tamanio = tamanio;
}

char* buscarPagina(uint32_t PID, uint32_t pagina)
{
	char* paginaBuscada = malloc(configuracion->tamano_pagina);
	t_nodoOcupado* nodo = encontrarNodoPorPID(espacioOcupado,PID);
	uint32_t ubicacionPagina = nodo->comienzo + pagina;
	memcpy(paginaBuscada,archivoMapeado->memoria + ubicacionPagina*configuracion->tamano_pagina,configuracion->tamano_pagina);
	uint32_t byteInicial = nodo->comienzo * configuracion->tamano_pagina;
	bool vacio = string_is_empty(paginaBuscada);
	uint32_t tamanio;
	if(vacio)
	{
		tamanio = 0;
	}
	else
	{
		tamanio = string_length(paginaBuscada);
	}
	if(tamanio != 0)
	{
	log_info(SwapLog,"Lectura solicitada por proceso con PID %d, byte inicial %d y tamanio %d y contenido: %s",PID,byteInicial,tamanio,paginaBuscada);
	}else
	{
		log_info(SwapLog,"Lectura solicitada por proceso con PID %d, byte inicial %d y tamanio %d y contenido: %s",PID,byteInicial,tamanio,"pagina vacia\0");
	}
	return paginaBuscada;
}

void escribirPagina(char* pagina,uint32_t PID,uint32_t ubicacion)
{
	t_nodoOcupado* nodo = encontrarNodoPorPID(espacioOcupado,PID);
	uint32_t posicionEnArchivo = (nodo->comienzo + ubicacion)*configuracion->tamano_pagina;
	char* areaMemoriaAEscribir = archivoMapeado->memoria + posicionEnArchivo;
	memcpy(areaMemoriaAEscribir, pagina,configuracion->tamano_pagina);
}

bool asignarProceso(t_mensaje* detalle)
{
	bool resultado;
	uint32_t posicionInicial,byteInicial,tamanio;

	resultado = hayEspacioSecuencial(detalle->paginas);
	//faltaria pensar el caso de compactacion
	if(resultado)
	{
		posicionInicial = ocuparEspacio(detalle->PID,detalle->paginas);
		agregarAEstadistica(detalle->PID);
		byteInicial = posicionInicial*configuracion->tamano_pagina;
		tamanio = detalle->paginas * configuracion->tamano_pagina;
		log_info(SwapLog,"Se asigna proceso con PID %d, byte inicial %d y tamaño %d",detalle->PID,byteInicial,tamanio);
	}
	else
	{
		log_info(SwapLog,"Proceso rechazado por falta de espacio PID:%d",detalle->PID);
	}
	return resultado;
}

void agregarAEstadistica(uint32_t PID)
{
	t_estadistica* nodo = malloc(sizeof(t_estadistica));
	nodo->PID = PID;
	nodo->escrituras = 0;
	nodo->lecturas = 0;
	list_add(estadisticasProcesos,nodo);
}

void aumentarEscritura(uint32_t PID)
{
	t_estadistica* nodo = encontrarNodoPorPID(estadisticasProcesos,PID);
	nodo->escrituras++;
}

void aumentarLectura(uint32_t PID)
{
	t_estadistica* nodo = encontrarNodoPorPID(estadisticasProcesos,PID);
	nodo->lecturas++;
}

void* encontrarNodoPorPID(t_list* lista, uint32_t PID)
{
	pidCondicion = PID;
	return list_find(lista,validarMismoPid);
}

void procesarInicio(t_mensaje* detalle,sock_t* socket)
{
	int32_t status;
	bool resultado;
	resultado = asignarProceso(detalle);
	int32_t mensaje = resultado;
	status = send(socket->fd, &mensaje, sizeof(int32_t),0);
	/*chequea envío*/
	if(!status)
	{
		printf("No se envió la cantidad de bytes a enviar luego\n");
	}
}

void procesarFinalizacion(t_mensaje* detalle,sock_t* socketMemoria)
{

	pidCondicion = detalle->PID;
	bool resultado = list_any_satisfy(espacioOcupado,validarMismoPid);
	if(resultado == 1)
	{
	liberarProceso(detalle->PID);
	}
	else
	{
		printf("Se intenta eliminar un proceso que no existe en memoria \n");
	}
	//Se envia 1 si salio todo bien y 0 en caso contrario.
	int32_t mensaje = resultado;
	int32_t status = send(socketMemoria->fd, &mensaje, sizeof(int32_t),0);
	if(!status)
	{
		printf("Irregularidad en el envio\n");
	}

}

void procesarLectura(t_mensaje* detalle,sock_t* socketMemoria)
{
	pidCondicion = detalle->PID;
	int32_t status;
	int32_t resultado;
	bool buscar = list_any_satisfy(espacioOcupado,validarMismoPid);
	resultado = buscar;
	if(buscar == 1)
	{
	char* pagina = buscarPagina(detalle->PID, detalle->ubicacion);
	int32_t tamanio,offset;
	char* mensaje;
	aumentarLectura(detalle->PID);
	//serializo mensaje, esto se puede mejorar.
	offset = 0;
	int32_t tamanioPagina = string_length(pagina) + 1;
	tamanio = sizeof(int32_t) + sizeof(int32_t) + tamanioPagina;
	mensaje = malloc(tamanio);
	memcpy(mensaje + offset, &resultado, sizeof(int32_t));
	offset = offset + sizeof(int32_t);
	memcpy(mensaje + offset, &tamanioPagina, sizeof(int32_t));
	offset = offset + sizeof(int32_t);
	if(tamanioPagina > 0)
	{
	memcpy(mensaje + offset, pagina, tamanioPagina);
	}
	send(socketMemoria->fd,mensaje,tamanio,0);
	free(mensaje);
	free(pagina);
	}
	else
	{
		printf("No existe el proceso en memoria \n");
		status = send(socketMemoria->fd, &resultado, sizeof(int32_t),0);
		/*chequea envío*/
		if(!status)
		{
			printf("No se envió la cantidad de bytes a enviar luego\n");
		}
	}
}
