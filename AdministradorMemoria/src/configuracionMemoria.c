//********************ARCHIVO DE CONFIGURACION*******************//
//****************************CPU***********************//

#include "administradorMemoria.h"

int32_t conf_es_valida(t_config * fd_configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "IP_SWAP")
		&& config_has_property(fd_configuracion, "PUERTO_SWAP")
		&& config_has_property(fd_configuracion, "MAXIMO_MARCOS_POR_PROCESO")
		&& config_has_property(fd_configuracion, "CANTIDAD_MARCOS")
		&& config_has_property(fd_configuracion, "TAMANIO_MARCO")
		&& config_has_property(fd_configuracion, "ENTRADAS_TLB")
		&& config_has_property(fd_configuracion, "TLB_HABILITADA")
		&& config_has_property(fd_configuracion, "RETARDO_MEMORIA")
		&& config_has_property(fd_configuracion, "ALGORITMO_REEMPLAZO")
	);
}

int cargarArchivoDeConfiguracion()
{

	fd_configuracion= config_create("src/configMemoria.txt");

	if (!conf_es_valida(fd_configuracion)) //ver que el archivo config este completo
	{
		//printf("Archivo de configuracion incompleto o invalido.\n");
		log_error(MemoriaLog,"Archivo de configuración inválido.","ERROR");
		return -1;
	}
	configuracion = malloc(sizeof(t_Memoria_Config));
	configuracion->puerto_escucha = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	configuracion->ip_swap = string_new();
	configuracion->ip_swap = config_get_string_value(fd_configuracion, "IP_SWAP");
	configuracion->puerto_swap = config_get_int_value(fd_configuracion, "PUERTO_SWAP");
	configuracion->maximo_marcos_por_proceso = config_get_int_value(fd_configuracion, "MAXIMO_MARCOS_POR_PROCESO");
	configuracion->cantidad_marcos = config_get_int_value(fd_configuracion, "CANTIDAD_MARCOS");
	configuracion->tamanio_marco = config_get_int_value(fd_configuracion, "TAMANIO_MARCO");
	configuracion->entradas_tlb = config_get_int_value(fd_configuracion, "ENTRADAS_TLB");
	configuracion->tlb_habilitada = config_get_int_value(fd_configuracion, "TLB_HABILITADA");
	configuracion->retardo_memoria = config_get_int_value(fd_configuracion, "RETARDO_MEMORIA");
	configuracion->algoritmo_reemplazo = string_new();
	configuracion->algoritmo_reemplazo = config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO");

	if(!string_equals_ignore_case(configuracion->algoritmo_reemplazo, FIFO) ||
	   !string_equals_ignore_case(configuracion->algoritmo_reemplazo, CLOCKM) ||
	   !string_equals_ignore_case(configuracion->algoritmo_reemplazo, LRU) ){

			log_error(MemoriaLog,"Algoritmo de reemplazo inválido.","ERROR");
			return -1;
	}

	printf("PUERTO_ESCUCHA: %d\n", configuracion->puerto_escucha);
	printf("IP_SWAP: %s\n", configuracion->ip_swap);
	printf("PUERTO_SWAP: %d\n", configuracion->puerto_swap);
	printf("MAXIMO_MARCOS_POR_PROCESO: %d\n", configuracion->maximo_marcos_por_proceso);
	printf("CANTIDAD_MARCOS: %d\n", configuracion->cantidad_marcos);
	printf("TAMANIO_MARCO: %d\n", configuracion->tamanio_marco);
	printf("ENTRADAS_TLB: %d\n", configuracion->entradas_tlb);
	printf("TLB_HABILITADA: %d\n", configuracion->tlb_habilitada);
	printf("RETARDO_MEMORIA: %d\n", configuracion->retardo_memoria);
	printf("ALGORITMO_REEMPLAZO: %s\n", configuracion->algoritmo_reemplazo);
	return 0;
}

void limpiarConfiguracion()
{
	config_destroy(fd_configuracion);
	free(configuracion);
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************CPU***************************//
