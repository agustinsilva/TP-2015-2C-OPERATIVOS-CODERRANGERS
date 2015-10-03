#ifndef ADMINSWAP_H_
#define ADMINSWAP_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/config.h>
#include <socket.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

//Estructuras
typedef struct {
	uint32_t puerto_escucha;
	char* nombre_swap;
	uint32_t cantidad_paginas;
	uint32_t tamano_pagina;
	uint32_t retardo_compactacion;
} t_configuracion;

//Comienzo de espacio libre y cantidad de paginas secuenciales libres.
typedef struct {
	uint32_t comienzo;
	uint32_t paginas;
}t_nodoLibre;

//id del proceso, comienzo del proceso en particion y paginas ocupadas.
typedef struct {
	uint32_t PID;
	uint32_t comienzo;
	uint32_t paginas;
}t_nodoOcupado;

typedef struct{
	uint32_t fd;
	char* memoria;
	uint32_t tamanio;
}t_archivoSwap;

typedef struct{
	uint32_t PID;
	uint32_t lecturas;
	uint32_t escrituras;
}t_estadistica;

typedef struct{
	uint32_t PID;
	uint32_t paginas;
	uint32_t ubicacion;
	uint32_t tamanioContenido;
	char* contenidoPagina;
}t_mensaje;

//Constantes
#define INICIAR 1
#define FINALIZAR 2
#define LEER 3
#define ESCRIBIR 4
#define ANORMAL 5

#define FALSO 0
#define VERDADERO 1

//Variables globales
t_configuracion* configuracion;
t_config* fd_configuracion;
t_log* SwapLog;
t_list* espacioLibre;
t_list* espacioOcupado;
t_list* estadisticasProcesos;
uint32_t paginasCondicion;  //cuidado con esta variable A.S.
uint32_t ubicacionCondicion;
uint32_t pidCondicion;
t_archivoSwap* archivoMapeado;

//Firma de funciones
int conf_es_valida(t_config*);
int cargarArchivoDeConfiguracion();
int32_t enviarMensaje(sock_t*, char*);
char* recibirMensaje(sock_t*);
void limpiarConfiguracion();
void crearParticion();
void eliminarParticion();
void inicializarParticion();
uint32_t contarPaginasLibres();
bool hayEspacio(uint32_t);
bool hayEspacioSecuencial(uint32_t);
bool validarEspacioLibre(void* nodo);
uint32_t ocuparEspacio(uint32_t,uint32_t);
bool validarUbicacionLibre(void*);
void liberarProceso(uint32_t);
bool validarMismoPid(void*);
char* buscarPagina(uint32_t, uint32_t);
void escribirPagina(char*,uint32_t,uint32_t);
void iniciarServidor();
void mappear_archivo();
uint32_t deserializarEnteroSinSigno(sock_t*);
t_mensaje* deserializarDetalle(sock_t*, uint32_t);
bool asignarProceso(t_mensaje*);
void agregarAEstadistica(uint32_t);
void aumentarEscritura(uint32_t);
void aumentarLectura(uint32_t);
void procesarInicio(t_mensaje*,sock_t*);
void* encontrarNodoPorPID(t_list*, uint32_t);
void liberarRecursos();
void procesarFinalizacion(t_mensaje*,sock_t*);
void procesarLectura(t_mensaje*,sock_t*);
void limpiarNodosLibres(void*);
void limpiarNodosOcupados(void*);
void limpiarEstadisticas(void*);
#endif /* ADMINSWAP_H_ */
