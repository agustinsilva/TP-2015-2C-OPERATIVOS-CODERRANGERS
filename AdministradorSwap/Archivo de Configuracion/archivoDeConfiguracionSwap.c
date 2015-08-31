//********************ARCHIVO DE CONFIGURACION*******************//
//****************************CPU***********************//

#include <commons/config.h>
#include <stdio.h>

int conf_es_valida(t_config * configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(configuracion, "NOMBRE_SWAP")
		&& config_has_property(configuracion, "CANTIDAD_PAGINAS")
		&& config_has_property(configuracion, "TAMANO_PAGINA")
		&& config_has_property(configuracion, "RETARDO_COMPACTACION"));
}

int cargarArchivoDeConfiguracion()

{
	t_config * configuracion;
	int puerto_escucha;
	char* nombre_swap;
	int cantidad_paginas;
	int tamano_pagina;
	int retardo_compactacion;

	configuracion= config_create("Archivo de Configuracion/configSwap.txt");
	
	if (!conf_es_valida(configuracion)) //ver que el archivo config este completo
	{
		puts("Archivo de configuracion incompleto o invalido.\n");
		return -2;
	}
       
	puerto_escucha = config_get_int_value(configuracion, "PUERTO_ESCUCHA");
	nombre_swap = config_get_string_value(configuracion, "NOMBRE_SWAP");
	cantidad_paginas = config_get_int_value(configuracion, "CANTIDAD_PAGINAS");
	tamano_pagina = config_get_int_value(configuracion, "TAMANO_PAGINA");
	retardo_compactacion = config_get_int_value(configuracion, "RETARDO_COMPACTACION");
	
	printf("PUERTO_ESCUCHA: %d\n", puerto_escucha);
	printf("NOMBRE_SWAP: %s\n", nombre_swap);
	printf("CANTIDAD_PAGINAS: %d\n", cantidad_paginas);
	printf("TAMANO_PAGINA: %d\n", tamano_pagina);
	printf("RETARDO_COMPACTACION: %d\n", retardo_compactacion);

	return 0;
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************CPU***************************//
