//********************ARCHIVO DE CONFIGURACION*******************//
//****************************SWAP***********************//

#include "administradorSwap.h"

int conf_es_valida(t_config* fd_configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "NOMBRE_SWAP")
		&& config_has_property(fd_configuracion, "CANTIDAD_PAGINAS")
		&& config_has_property(fd_configuracion, "TAMANO_PAGINA")
		&& config_has_property(fd_configuracion, "RETARDO_COMPACTACION")
		&& config_has_property(fd_configuracion, "RETARDO_SWAP"));
}

int cargarArchivoDeConfiguracion()
{
	configuracion = malloc(sizeof(t_configuracion));
	fd_configuracion= config_create("src/configSwap.txt");
	if (!conf_es_valida(fd_configuracion)) //ver que el archivo config este completo
	{
		//puts("Archivo de configuracion incompleto o invalido.\n");
		log_error(SwapLog,"Archivo de configuración inválido.","ERROR");
		return -2;
	}
	configuracion->puerto_escucha = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	configuracion->nombre_swap = config_get_string_value(fd_configuracion, "NOMBRE_SWAP");
	configuracion->cantidad_paginas = config_get_int_value(fd_configuracion, "CANTIDAD_PAGINAS");
	configuracion->tamano_pagina = config_get_int_value(fd_configuracion, "TAMANO_PAGINA");
	configuracion->retardo_compactacion = config_get_int_value(fd_configuracion, "RETARDO_COMPACTACION");
	configuracion->retardo_swap = config_get_int_value(fd_configuracion, "RETARDO_SWAP");

	log_info(SwapLog,
		"\nPUERTO_ESCUCHA: %d\n"
		"NOMBRE_SWAP: %s\n"
		"CANTIDAD_PAGINAS: %d\n"
		"TAMANIO_PAGINA: %d\n"
		"RETARDO_COMPACTACION: %d\n"
		"RETARDO_SWAP: %d\n" ,
		configuracion->puerto_escucha, configuracion->nombre_swap , configuracion->cantidad_paginas ,
		configuracion->tamano_pagina, configuracion->retardo_compactacion , configuracion->retardo_swap);
	return 0;
}

void limpiarConfiguracion()
{
	config_destroy(fd_configuracion);
	free(configuracion);
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************SWAP***************************//
