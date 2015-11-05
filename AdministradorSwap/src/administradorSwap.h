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
	uint32_t retardo_swap;
} t_configuracion;

//Comienzo de espacio libre y cantidad de paginas secuenciales libres.
typedef struct {
	uint32_t comienzo;
	uint32_t paginas;
}t_nodoLibre;

//id del proceso, comienzo del proceso en particion y paginas ocupadas.
typedef struct {
	int32_t PID;
	uint32_t comienzo;
	uint32_t paginas;
}t_nodoOcupado;

typedef struct{
	uint32_t fd;
	char* memoria;
	uint32_t tamanio;
}t_archivoSwap;

typedef struct{
	int32_t PID;
	uint32_t lecturas;
	uint32_t escrituras;
}t_estadistica;

typedef struct{
	int32_t PID;
	int32_t CantidadPaginas; //paginas
	int32_t NumeroDePagina; //ubicacion
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

#define RESET "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define HIDDEN "\e[8m"
#define BOLD "\e[1m"
#define UNDERLINED "\e[4m"
#define RESET_NON_BOLD "\e[21m"
#define RESET_NON_UL "\e[24m"
#define BLINK "\e[5m"
#define RESET_NON_BLINK "\e[25m"

//Variables globales
t_configuracion* configuracion;
t_config* fd_configuracion;
t_log* SwapLog;
t_list* espacioLibre;
t_list* espacioOcupado;
t_list* estadisticasProcesos;
int32_t CantidadDePaginasCondicion;  //cuidado con esta variable A.S.
int32_t NumeroDePaginaCondicion;
int32_t pidCondicion;
t_archivoSwap* archivoMapeado;

//Firma de funciones
int conf_es_valida(t_config*);
int cargarArchivoDeConfiguracion();
int32_t enviarMensaje(sock_t*, char*);
/*char* recibirMensaje(sock_t*,int32_t);*/
void limpiarConfiguracion();
void crearParticion();
void eliminarParticion();
void inicializarParticion();
uint32_t contarPaginasLibres();
bool hayEspacio(int32_t);
bool hayEspacioSecuencial(int32_t);
bool validarEspacioLibre(void* nodo);
uint32_t ocuparEspacio(int32_t,int32_t);
bool validarUbicacionLibre(void*);
void liberarProceso(int32_t);
bool validarMismoPid(void*);
char* buscarPagina(int32_t, int32_t);
void escribirPagina(char*,int32_t,int32_t);
void iniciarServidor();
void mappear_archivo();
int32_t deserializarEntero(sock_t*);
t_mensaje* deserializarDetalle(sock_t*, int32_t);
bool asignarProceso(t_mensaje*);
void agregarAEstadistica(int32_t);
void aumentarEscritura(int32_t);
void aumentarLectura(int32_t);
void procesarInicio(t_mensaje*,sock_t*);
void* encontrarNodoPorPID(t_list*, int32_t);
void liberarRecursos();
void procesarFinalizacion(t_mensaje*,sock_t*);
void procesarLectura(t_mensaje*,sock_t*);
void procesarEscritura(t_mensaje*,sock_t*);
void limpiarNodosLibres(void*);
void limpiarNodosOcupados(void*);
void limpiarEstadisticas(void*);
bool buscarNodoComienzo(void*);
void compactacionBruta();
bool compararUbicaciones(void*,void*);
void tituloInicial();
#endif /* ADMINSWAP_H_ */
