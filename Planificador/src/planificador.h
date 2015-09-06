#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <socket.h>
#include <commons/string.h>
//Estructuras
typedef struct {
	uint32_t puertoEscucha;
	char* algoritmoPlanificacion;
	uint32_t quantum;
}t_configuracion;

//Constantes
#define PAQUETE 1024
//Variables globales
t_configuracion configuracion;

//Firma de funciones
void* iniciarServidor();
int conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();

#endif /* PLANIFICADOR_H_ */
