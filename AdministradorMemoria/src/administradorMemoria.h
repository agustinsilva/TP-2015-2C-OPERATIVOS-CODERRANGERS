#ifndef ADMINMEMORIA_H_
#define ADMINMEMORIA_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <commons/config.h>
#include <commons/string.h>
#include <socket.h>

//Estructuras
typedef struct{
	int32_t puerto_escucha;
	char* ip_swap;
	int32_t puerto_swap;
	int32_t maximo_marcos_por_proceso;
	int32_t cantidad_marcos;
	int32_t tamanio_marco;
	int32_t entradas_tlb;
	int32_t tlb_habilitada;
	int32_t retardo_memoria;
} t_Memoria_Config;

//Constantes

//Variables globales
t_Memoria_Config* configuracion;

//Firma de funciones
char* recibirMensaje(sock_t* socket);
int32_t enviarMensaje(sock_t* socket, char* mensaje);
int32_t conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();

#endif /* ADMINMEMORIA_H_ */
