#ifndef CPU_H_
#define CPU_H_
#define TAMINSTRUCCION 80
#define iniciar 1
#define finalizar 5

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


typedef struct pcb{
		char* path;
}t_pcb;

//Constantes
#define PAQUETE 1024

//Variables globales
t_configuracion* configuracion;
t_config * fd_configuracion;
t_log* CPULog;

//Firma de funciones
int conf_es_valida(t_config* configuracion);
int cargarArchivoDeConfiguracion();
void* ConectarAPlanificador();
t_pcb escucharPlanificador();
void escucharYAtender();
void limpiarConfiguracion();
int informarAdminMemoriaComandoIniciar(char* cantidadPaginas);
int informarAdminMemoriaComandoFinalizar(char * path);
void crearHilosCPU (void);
#endif /* CPU_H_ */
