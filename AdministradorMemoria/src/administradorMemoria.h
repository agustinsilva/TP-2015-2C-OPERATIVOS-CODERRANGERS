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
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <socket.h>
#include <commons/log.h>
#include <pthread.h>

// Constantes
#define codigoIniciar 1
#define codigoFinalizar 2
#define codigoLeer 3

//Estructuras
typedef struct{
	uint32_t puerto_escucha;
	char* ip_swap;
	int32_t puerto_swap;
	int32_t maximo_marcos_por_proceso;
	int32_t cantidad_marcos;
	int32_t tamanio_marco;
	int32_t entradas_tlb;
	int32_t tlb_habilitada;
	int32_t retardo_memoria;
} t_Memoria_Config;

typedef struct{
	sock_t* cpuSocket;
	sock_t* swapSocket;
} t_HiloCPU;

typedef struct{
	int32_t	dirLogica;
	char*	dirFisica;
} t_TLB;

typedef struct{
	int32_t	marco;
	int32_t	pagina;
} t_MP;


//Variables globales
t_Memoria_Config* configuracion;
t_config * fd_configuracion;
t_log* MemoriaLog;
t_list* TLB;
t_list* memoriaPrincipal;


//Firma de funciones
char* recibirMensaje(sock_t* socket);
int32_t enviarMensaje(sock_t* socket, char* mensaje);
int32_t conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();
void limpiarConfiguracion();
void* hiloEjecucionCPU(t_HiloCPU* );
void setUp();
void limpiarMemoriaPrincipal();
void limpiarTLB();

#endif /* ADMINMEMORIA_H_ */
