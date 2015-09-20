#ifndef ADMINSWAP_H_
#define ADMINSWAP_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
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

//Constantes

//Variables globales
t_configuracion* configuracion;
t_config* fd_configuracion;
t_log* SwapLog;
t_list* espacioLibre;
t_list* espacioOcupado;
uint32_t paginasCondicion;  //cuidado con esta variable A.S.
uint32_t ubicacionCondicion;
uint32_t pidCondicion;
t_archivoSwap* archivoMapeado;

//Firma de funciones
int conf_es_valida(t_config* fd_configuracion);
int cargarArchivoDeConfiguracion();
int32_t enviarMensaje(sock_t* socket, char* mensaje);
char* recibirMensaje(sock_t* socket);
void limpiarConfiguracion();
void crearParticion();
void eliminarParticion();
void inicializarParticion();
uint32_t contarPaginasLibres();
short hayEspacio(uint32_t paginas);
bool hayEspacioSecuencial(uint32_t paginas);
bool validarEspacioLibre(void* nodo);
void ocuparEspacio(uint32_t PID,uint32_t paginasAOcupar);
bool validarUbicacionLibre(void* nodo);
void liberarEspacio(uint32_t PID);
bool validarMismoPid(void* nodo);
char* buscarPagina(uint32_t PID, uint32_t pagina);
void escribirPagina(char* pagina,uint32_t PID,uint32_t ubicacion);
void iniciarServidor();
void mappear_archivo();
#endif /* ADMINSWAP_H_ */
