//********************ARCHIVO DE CONFIGURACION*******************//
//****************************CPU***********************//

#include <commons/config.h>
#include <stdio.h>

int conf_es_valida(t_config * configuracion)
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

int cargarArchivoDeConfiguracion()
{
	t_config * configuracion;
	int puerto_escucha;
	char* ip_swap;
	int puerto_swap;
	int maximo_marcos_por_proceso;
	int cantidad_marcos;
	int tamanio_marco;
	int entradas_tlb;
	int tlb_habilitada;
	int retardo_memoria;

	configuracion= config_create("Archivo de Configuracion/configMemoria.txt");
	
	if (!conf_es_valida(configuracion)) //ver que el archivo config este completo
	{
		puts("Archivo de configuracion incompleto o invalido.\n");
		return -2;
	}
       
	puerto_escucha = config_get_int_value(configuracion, "PUERTO_ESCUCHA");
	ip_swap = config_get_string_value(configuracion, "IP_SWAP");
	puerto_swap = config_get_int_value(configuracion, "PUERTO_SWAP");
	maximo_marcos_por_proceso = config_get_int_value(configuracion, "MAXIMO_MARCOS_POR_PROCESO");
	cantidad_marcos = config_get_int_value(configuracion, "CANTIDAD_MARCOS");
	tamanio_marco = config_get_int_value(configuracion, "TAMANIO_MARCO");
	entradas_tlb = config_get_int_value(configuracion, "ENTRADAS_TLB");
	tlb_habilitada = config_get_int_value(configuracion, "TLB_HABILITADA");
	retardo_memoria = config_get_int_value(configuracion, "RETARDO_MEMORIA");
	
	printf("PUERTO_ESCUCHA: %d\n", puerto_escucha);
	printf("IP_SWAP: %s\n", ip_swap);
	printf("PUERTO_SWAP: %d\n", puerto_swap);
	printf("MAXIMO_MARCOS_POR_PROCESO: %d\n", maximo_marcos_por_proceso);
	printf("CANTIDAD_MARCOS: %d\n", cantidad_marcos);
	printf("TAMANIO_MARCO: %d\n", tamanio_marco);
	printf("ENTRADAS_TLB: %d\n", entradas_tlb);
	printf("TLB_HABILITADA: %d\n", tlb_habilitada);
	printf("RETARDO_MEMORIA: %d\n", retardo_memoria);

	return 0;
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************CPU***************************//
