#ifndef ADMINSWAP_H_
#define ADMINSWAP_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <socket.h>
#include <commons/string.h>
#include <commons/log.h>

//Estructuras
typedef struct {
	u_int32_t puerto_escucha;
	char* nombre_swap;
	u_int32_t cantidad_paginas;
	u_int32_t tamano_pagina;
	u_int32_t retardo_compactacion;
} t_configuracion;

//Constantes

//Variables globales
t_configuracion* configuracion;
t_config* fd_configuracion;
t_log* SwapLog;

//Firma de funciones
int conf_es_valida(t_config * fd_configuracion);
int cargarArchivoDeConfiguracion();
int32_t enviarMensaje(sock_t* socket, char* mensaje);
char* recibirMensaje(sock_t* socket);
void limpiarConfiguracion();
void crearParticion();
void eliminarParticion();

#endif /* ADMINSWAP_H_ */
