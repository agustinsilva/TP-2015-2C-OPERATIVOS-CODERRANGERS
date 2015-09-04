#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

//Estructuras
typedef struct {
	int puertoEscucha;
	char* algoritmoPlanificacion;
	int quantum;
}t_configuracion;

//Constantes

//Variables globales
t_configuracion configuracion;

//Firma de funciones
void* iniciarServidor();
int conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();

#endif /* PLANIFICADOR_H_ */
