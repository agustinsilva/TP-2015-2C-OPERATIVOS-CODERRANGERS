//*********************************************************************//
//********************HEADERS-FUNCIONES-ARCH*********************//
//*******************************************************************//


#ifndef CFGADMINMEMORIA_H_
#define CFGADMINMEMORIA_H_

#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>

int32_t conf_es_valida(t_config * configuracion);
t_Memoria_Config* cargarArchivoDeConfiguracion();

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

#endif /* CFGADMINMEMORIA_H_ */
