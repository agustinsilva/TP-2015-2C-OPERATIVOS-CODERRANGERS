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
#include <time.h>

//Estructuras
typedef struct {
	uint32_t puertoEscucha;
	char* algoritmoPlanificacion;
	int32_t quantum;
}t_configuracion;

typedef struct {
	uint32_t idProceso;
	uint32_t estadoProceso; //0-Espera 1-Ejecucion 2-Finalizado 3-Bloqueado
	uint32_t contadorPuntero;
	uint32_t cantidadInstrucciones;
	uint32_t tamaniopath;
	char* path;
	uint32_t retardo;
	uint32_t flagFin;
	time_t tiempoCreacion;
	time_t tiempoEjecucion;
	time_t tiempoEspera;
}t_pcb;

typedef struct {
	uint32_t socketHilo;
	uint32_t estadoHilo; //0-disponible 1-Ejecutando
	char* path;
	int32_t idProceso;
}t_hilosConectados;

typedef struct{
	uint32_t idProceso;
	double tiempoRespuesta;//Tiempo desde que empieza hasta que termina.
	double tiempoEjecucion;//Tiempo que esta en la CPU.
	double tiempoEspera;//Tiempo que esa en cola de ready.
}t_proc_metricas;

//Variables globales
t_configuracion* configuracion; //Puntero a configuracion
t_config* fdConfiguracion; //Descriptor de archivo
int contadorProceso;
int32_t socketCpuPadre;
t_list *proc_listos;
t_list *proc_ejecutados;
t_list *proc_bloqueados;
t_list *cpu_listos;
t_list *cpu_ocupados;
t_list *proc_metricas;
sem_t sincroproc;
sem_t sincrocpu;
sem_t sincroBloqueados;
sem_t mutex;
t_log* planificadorLog;

//Constantes
#define PAQUETE 1024
#define AGREGARPADRECPU 10
#define ESTADOCPUPADRE 11
#define AGREGARHILOCPU 1
#define LOGEARRESULTADOCPU 2
#define LOGEARFINALIZACIONCPU 3
#define FINQUANTUM 4
#define ENTRADASALIDA 5
#define ENVIARPCB 2
#define ERROR 5

//Codigo de configuracion inicial enviada por el planificador.
#define CFG_INICIAL_PLN 26

//Colores Consola
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define BOLD "\e[1m"
#define RESET_NON_BOLD "\e[21m"

//Firma de funciones
void* iniciarServidor();
void encolar(char*);
void consumirRecursos();
void pcbDestroy(t_pcb*);
void replanificar(int32_t );
void bloquearProceso(int32_t , uint32_t*);
void finalizarProceso(uint32_t*);
t_pcb* recibirPcb(uint32_t );
void iniciarHiloBloqueados();
void creoPadre(int32_t);
void enviarTipoPlanificacion();
void logearResultadoCpu(int32_t);
void logearFinalizacionCpu(int32_t);
char* serializarPCB(t_pcb*, uint32_t*);
void generoHiloPlanificador(uint32_t*);
void creoCpu(int32_t);
int contarInstrucciones(char*);
char* convertirNumeroEnString(uint32_t);
void mostrarProcesos();
void* mostrarConsola();
void pedirEstadoCpu();
void leerComando(uint32_t*, char*);
int conf_es_valida(t_config *);
int cargarArchivoDeConfiguracion();
int32_t crearSocketReceptor();
int32_t deserializarEntero(int32_t);
void limpiarConfiguracion();
void inicializar();
void killThemAll();
char* recibirMensaje(int32_t);
void tituloInicial();
double calculoDiferenciaTiempoActual(time_t);
void mostrarMetricas();
void enviarEnteros(int32_t, int32_t);
#endif /* PLANIFICADOR_H_ */
