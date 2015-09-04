//********************ARCHIVO DE CONFIGURACION*******************//
//****************************CPU***********************//

#include "funcionesConfig.h"

int32_t conf_es_valida(t_config * configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(configuracion, "IP_SWAP")
		&& config_has_property(configuracion, "PUERTO_SWAP")
		&& config_has_property(configuracion, "MAXIMO_MARCOS_POR_PROCESO")
		&& config_has_property(configuracion, "CANTIDAD_MARCOS")
		&& config_has_property(configuracion, "TAMANIO_MARCO")
		&& config_has_property(configuracion, "ENTRADAS_TLB")
		&& config_has_property(configuracion, "TLB_HABILITADA")
		&& config_has_property(configuracion, "RETARDO_MEMORIA")
	);
}

t_Memoria_Config* cargarArchivoDeConfiguracion()
{
	t_config * configuracion;
	t_Memoria_Config* memoriaConfig = malloc(sizeof(t_Memoria_Config));

	configuracion= config_create("ArchivoDeConfiguracion/configMemoria.txt");
	
	if (!conf_es_valida(configuracion)) //ver que el archivo config este completo
	{
		puts("Archivo de configuracion incompleto o invalido.\n");
		return NULL;
	}
       
	memoriaConfig->puerto_escucha = config_get_int_value(configuracion, "PUERTO_ESCUCHA");
	memoriaConfig->ip_swap = string_new();
	memoriaConfig->ip_swap = config_get_string_value(configuracion, "IP_SWAP");
	memoriaConfig->puerto_swap = config_get_int_value(configuracion, "PUERTO_SWAP");
	memoriaConfig->maximo_marcos_por_proceso = config_get_int_value(configuracion, "MAXIMO_MARCOS_POR_PROCESO");
	memoriaConfig->cantidad_marcos = config_get_int_value(configuracion, "CANTIDAD_MARCOS");
	memoriaConfig->tamanio_marco = config_get_int_value(configuracion, "TAMANIO_MARCO");
	memoriaConfig->entradas_tlb = config_get_int_value(configuracion, "ENTRADAS_TLB");
	memoriaConfig->tlb_habilitada = config_get_int_value(configuracion, "TLB_HABILITADA");
	memoriaConfig->retardo_memoria = config_get_int_value(configuracion, "RETARDO_MEMORIA");
	
	printf("PUERTO_ESCUCHA: %d\n", memoriaConfig->puerto_escucha);
	printf("IP_SWAP: %s\n", memoriaConfig->ip_swap);
	printf("PUERTO_SWAP: %d\n", memoriaConfig->puerto_swap);
	printf("MAXIMO_MARCOS_POR_PROCESO: %d\n", memoriaConfig->maximo_marcos_por_proceso);
	printf("CANTIDAD_MARCOS: %d\n", memoriaConfig->cantidad_marcos);
	printf("TAMANIO_MARCO: %d\n", memoriaConfig->tamanio_marco);
	printf("ENTRADAS_TLB: %d\n", memoriaConfig->entradas_tlb);
	printf("TLB_HABILITADA: %d\n", memoriaConfig->tlb_habilitada);
	printf("RETARDO_MEMORIA: %d\n", memoriaConfig->retardo_memoria);

	return memoriaConfig;
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************CPU***************************//
