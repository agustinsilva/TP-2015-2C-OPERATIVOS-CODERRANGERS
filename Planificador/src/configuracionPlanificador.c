//********************ARCHIVO DE CONFIGURACION*******************//
//****************************PLANIFICADOR***********************//

#include "planificador.h"

int conf_es_valida(t_config* fdConfiguracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(fdConfiguracion, "PUERTO_ESCUCHA")
		&& config_has_property(fdConfiguracion, "ALGORITMO_PLANIFICACION")
		&& config_has_property(fdConfiguracion, "QUANTUM"));
}

int cargarArchivoDeConfiguracion()

{
	configuracion = malloc(sizeof(t_configuracion));

	fdConfiguracion= config_create("src/configPlanificador.txt");
	
	if (!conf_es_valida(fdConfiguracion)) //ver que el archivo de config tenga todo
	{
		puts("Archivo de configuracion incompleto o invalido.\n");
		return -2;
	}
       
	configuracion->puertoEscucha = config_get_int_value(fdConfiguracion, "PUERTO_ESCUCHA");
	configuracion->algoritmoPlanificacion = config_get_string_value(fdConfiguracion, "ALGORITMO_PLANIFICACION");
	configuracion->quantum = config_get_int_value(fdConfiguracion, "QUANTUM");

	log_info(planificadorLog,
		"\nPUERTO_ESCUCHA: %d\n"
		"ALGORITMO_PLANIFICACION: %s\n"
		"QUANTUM: %d\n",
		configuracion->puertoEscucha, configuracion->algoritmoPlanificacion , configuracion->quantum);
	return 0;
}

void limpiarConfiguracion()
{
	config_destroy(fdConfiguracion);
	free(configuracion);
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************PLANIFICADOR***************************//
