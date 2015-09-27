#ifndef CPU_H_
#define CPU_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <socket.h>
#include <commons/log.h>
#include <pthread.h>


//Estructuras
typedef struct {
	char* ipPlanificador;
	uint32_t puertoPlanificador;
	char* ipMemoria;
	uint32_t puertoMemoria;
	uint32_t cantidadHilos;
	uint32_t retardo;
}t_configuracion;

typedef struct {
  unsigned long int ID_CPU;
  int ESTADO;
}t_estructuraCPU;

typedef struct {
	uint32_t idProceso;
	uint32_t estadoProceso; //0-Espera 1-Ejecucion 2-Finalizado
	uint32_t contadorPuntero;
	uint32_t cantidadInstrucciones;
	char* path;
}t_pcb;

typedef struct {
 int length;
 char *data;
} t_stream;

//Constantes
#define PAQUETE 1024
#define TAMINSTRUCCION 80
#define INICIAR 1
#define RECIBIR_PCB 2
#define LEER 3
#define ESCRIBIR 4
#define ANORMAL 5
#define NUEVO_HILO 1

//Variables globales
t_configuracion* configuracion;
t_config * fd_configuracion;
t_log* CPULog;

//Firma de funciones
int conf_es_valida(t_config* configuracion);
int cargarArchivoDeConfiguracion();
void* ConectarAPlanificador();
t_pcb* escucharPlanificador();
void escucharYAtender();
void limpiarConfiguracion();
int informarAdminMemoriaComandoIniciar(char* cantidadPaginas);
int informarAdminMemoriaComandoFinalizar(char * path);
void crearHilosCPU (void);
uint32_t deserializarEnteroSinSigno(sock_t* socket);
t_pcb deserializarDetalle(sock_t* socket, uint32_t cabecera);
t_pcb* pcb_deserializar(t_stream* stream);
#endif /* CPU_H_ */
