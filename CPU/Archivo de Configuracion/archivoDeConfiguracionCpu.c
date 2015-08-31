//********************ARCHIVO DE CONFIGURACION*******************//
//****************************CPU***********************//

#include <commons/config.h>
#include <stdio.h>

int conf_es_valida(t_config * configuracion)
{
	// config_has_property Retorna true si key se encuentra en la configuracion.
	return (config_has_property(configuracion, "IP_PLANIFICADOR")
		&& config_has_property(configuracion, "PUERTO_PLANIFICADOR")
		&& config_has_property(configuracion, "IP_MEMORIA")
		&& config_has_property(configuracion, "PUERTO_MEMORIA")
		&& config_has_property(configuracion, "CANTIDAD_HILOS")
		&& config_has_property(configuracion, "RETARDO"));
}

int cargarArchivoDeConfiguracion()

{
	t_config * configuracion;
	char* ip_planificador;
	int puerto_planificador;
	char* ip_memoria;
	int puerto_memoria;
	int cantidad_hilos;
	int retardo;

	configuracion= config_create("Archivo de Configuracion/configCPU.txt");
	
	if (!conf_es_valida(configuracion)) //ver que el archivo config este completo
	{
		puts("Archivo de configuracion incompleto o invalido.\n");
		return -2;
	}
       
	ip_planificador = config_get_string_value(configuracion, "IP_PLANIFICADOR");
	puerto_planificador = config_get_int_value(configuracion, "PUERTO_PLANIFICADOR");
	ip_memoria = config_get_string_value(configuracion, "IP_MEMORIA");
	puerto_memoria = config_get_int_value(configuracion, "PUERTO_MEMORIA");
	cantidad_hilos = config_get_int_value(configuracion, "CANTIDAD_HILOS");
	retardo = config_get_int_value(configuracion, "RETARDO");
	
	printf("IP_PLANIFICADOR: %s\n", ip_planificador);
	printf("PUERTO_PLANIFICADOR: %d\n", puerto_planificador);
	printf("IP_MEMORIA: %s\n", ip_memoria);
	printf("PUERTO_MEMORIA: %d\n", puerto_memoria);
	printf("CANTIDAD_HILOS: %d\n", cantidad_hilos);
	printf("RETARDO: %d segundos\n", retardo);

	return 0;
}

//***************************FIN********************************//
//*******************ARCHIVO DE CONFIGURACION*******************//
//***********************CPU***************************//
