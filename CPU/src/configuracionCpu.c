//********************ARCHIVO DE CONFIGURACION*******************//
//****************************CPU***********************//

#include "cpu.h"

int conf_es_valida(t_config* fd_configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(fd_configuracion, "IP_PLANIFICADOR")
		&& config_has_property(fd_configuracion, "PUERTO_PLANIFICADOR")
		&& config_has_property(fd_configuracion, "IP_MEMORIA")
		&& config_has_property(fd_configuracion, "PUERTO_MEMORIA")
		&& config_has_property(fd_configuracion, "CANTIDAD_HILOS")
		&& config_has_property(fd_configuracion, "RETARDO"));
}

int cargarArchivoDeConfiguracion()

{
	puts("Cargo archivo de configuracion de CPU");
	configuracion = malloc(sizeof(t_configuracion));
	fd_configuracion= config_create("src/configCPU.txt");
	
	if (!conf_es_valida(fd_configuracion)) //ver que el archivo config este completo
	{
		log_error(CPULog,"Archivo de configuración inválido.","ERROR");
		//puts("Archivo de configuracion incompleto o invalido.\n");
		return -2;
	}
       
	configuracion->ipPlanificador = config_get_string_value(fd_configuracion, "IP_PLANIFICADOR");
	configuracion->puertoPlanificador = config_get_int_value(fd_configuracion, "PUERTO_PLANIFICADOR");
	configuracion->ipMemoria = config_get_string_value(fd_configuracion, "IP_MEMORIA");
	configuracion->puertoMemoria = config_get_int_value(fd_configuracion, "PUERTO_MEMORIA");
	configuracion->cantidadHilos = config_get_int_value(fd_configuracion, "CANTIDAD_HILOS");
	configuracion->retardo = config_get_int_value(fd_configuracion, "RETARDO");
	
	log_info(CPULog,
		"\nIP_PLANIFICADOR: %s\n"
		"PUERTO_PLANIFICADOR: %d\n"
		"IP_MEMORIA: %s\n"
		"PUERTO_MEMORIA: %d\n"
		"CANTIDAD_HILOS: %d\n"
		"RETARDO: %d\n",
		configuracion->ipPlanificador, configuracion->puertoPlanificador , configuracion->ipMemoria ,
		configuracion->puertoMemoria, configuracion->cantidadHilos , configuracion->retardo);
	return 0;
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************CPU***************************//
