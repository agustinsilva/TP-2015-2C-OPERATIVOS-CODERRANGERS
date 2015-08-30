//********************ARCHIVO DE CONFIGURACION*******************//
//****************************PLANIFICADOR***********************//

#include <commons/config.h>
#include <stdio.h>

int conf_es_valida(t_config * configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(configuracion, "ALGORITMO_PLANIFICACION")
		&& config_has_property(configuracion, "QUANTUM"));
}

int cargarArchivoDeConfiguracion()

{
	t_config * configuracion;
	int puerto_escucha;
	char* algoritmo_planificacion;
	int quantum;

	configuracion= config_create("Archivo de configuracion/configPlanificador.txt");
	
	if (!conf_es_valida(configuracion)) //ver que el archivo de config tenga todo
	{
		puts("Archivo de configuracion incompleto o invalido.\n");
		return -2;
	}
       
	puerto_escucha = config_get_int_value(configuracion, "PUERTO_ESCUCHA");
	algoritmo_planificacion = config_get_string_value(configuracion, "ALGORITMO_PLANIFICACION");
	quantum = config_get_int_value(configuracion, "QUANTUM");
	
	printf("PUERTO_ESCUCHA:%d\n", puerto_escucha);
	printf("ALGORITMO_PLANIFICACION:%s\n", algoritmo_planificacion);
	printf("QUANTUM:%d\n", quantum);

	return 0;
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************PLANIFICADOR***************************//
