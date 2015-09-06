#ifndef ADMINSWAP_H_
#define ADMINSWAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <socket.h>
#include <commons/string.h>

typedef struct {
	u_int32_t puerto_escucha;
	char* nombre_swap;
	u_int32_t cantidad_paginas;
	u_int32_t tamano_pagina;
	u_int32_t retardo_compactacion;
} t_configuracion;

t_configuracion configuracion;

int conf_es_valida(t_config * td_configuracion);
int cargarArchivoDeConfiguracion();
int32_t enviarMensaje(sock_t* socket, char* mensaje);
char* recibirMensaje(sock_t* socket);

#endif /* ADMINSWAP_H_ */
