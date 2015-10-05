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
#include <commons/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/string.h>
#include <commons/collections/list.h>

//Estructuras
typedef struct {
	uint32_t puertoEscucha;
	char* algoritmoPlanificacion;
	uint32_t quantum;
}t_configuracion;

typedef struct {
	uint32_t idProceso;
	uint32_t estadoProceso; //0-Espera 1-Ejecucion 2-Finalizado
	uint32_t contadorPuntero;
	uint32_t cantidadInstrucciones;
	uint32_t tamaniopath;
	char* path;
}t_pcb;

typedef struct {
	uint32_t socketHilo;
	uint32_t estadoHilo; //0-disponible 1-Ejecutando
	char* path;
	uint32_t idProceso;
}t_hilosConectados;

//Variables globales
t_configuracion* configuracion; //Puntero a configuracion
t_config* fdConfiguracion; //Descriptor de archivo
int contadorProceso;
t_list *proc_listos;
t_list *proc_ejecutados;
t_list *cpu_listos;
t_list *cpu_ocupados;
sem_t sincroproc;
sem_t sincrocpu;
sem_t mutex;
t_log* planificadorLog;

//Constantes
#define PAQUETE 1024
#define AGREGARPADRECPU 0
#define AGREGARHILOCPU 1
#define LOGEARRESULTADOCPU 2
#define LOGEARFINALIZACIONCPU 3
#define ENVIARPCB 2
#define ERROR 5

//Colores Consola
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//Firma de funciones
void* iniciarServidor();
void encolar(char* path);
void consumirRecursos();
void creoPadre(socketProcesado);
void logearResultadoCpu(uint32_t socketCpu);
void logearFinalizacionCpu(uint32_t socketCpu);
char* serializarPCB(t_pcb *pcb, uint32_t *totalPaquete);
void generoHiloPlanificador(uint32_t *hiloCreado);
void creoCpu(uint32_t socketCpu);
int contarInstrucciones(char* path);
char* convertirNumeroEnString(uint32_t estado);
void mostrarProcesos();
void* mostrarConsola();
void leerComando(int* comando, char* mensaje);
int conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();
uint32_t crearSocketReceptor();
uint32_t deserializarEnteroSinSigno(uint32_t socket);
void limpiarConfiguracion();
char* recibirMensaje(uint32_t socket);
#endif /* PLANIFICADOR_H_ */
