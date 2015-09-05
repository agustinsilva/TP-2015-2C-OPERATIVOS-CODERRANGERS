#ifndef ADMINMEMORIA_H_
#define ADMINMEMORIA_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sockets/socket.h>

int32_t conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();

typedef struct{
	int32_t puerto_escucha;
	char* ip_swap;
	int32_t puerto_swap;
	int32_t maximo_marcos_por_proceso;
	int32_t cantidad_marcos;
	int32_t tamanio_marco;
	int32_t entradas_tlb;
	int32_t tlb_habilitada;
	int32_t retardo_memoria;
} t_Memoria_Config;

t_Memoria_Config* configuracion;

#endif /* ADMINMEMORIA_H_ */
