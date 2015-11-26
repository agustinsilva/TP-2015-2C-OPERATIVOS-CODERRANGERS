#ifndef CPU_H_
#define CPU_H_

//Inclusiones
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
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
#include <commons/process.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/time.h>

//Estructuras

typedef struct {
	uint32_t numeroCPU;
	pthread_t idCPU;
	uint32_t porcentajeProcesado;
	uint32_t tiempoAcumuladoDeInstrucciones;
}t_CPUsConectados;


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
	uint32_t tipoPlanificacion;
	uint32_t quantum;
}t_cpu_padre;


//Constantes
#define PAQUETE 1024
#define TAMINSTRUCCION 300
#define ANORMAL 5
#define PEDIDO_ERROR 0
/*con admin de memoria*/
#define INICIAR 1
#define FINALIZAR 2
#define LEER 3
#define ESCRIBIR 4
/*con planificador*/
#define FIN_QUANTUM 4
#define ENTRADA_SALIDA 5
#define RESPUESTA_PLANIFICADOR_LOGEAR 2
#define RESPUESTA_PLANIFICADOR_FIN_EJECUCION 3
#define NUEVO_HILO 1
#define CONEXION_CPU_PADRE 10
#define PORCENTAJES_CPU 11

#define PORCENTAJE 100
#define TIEMPO_MINUTO 60

//CONSOLA
#define BOLD "\e[1m"
#define RESET_NON_BOLD "\e[21m"

//Variables globales
t_configuracion* configuracion;
t_config * fd_configuracion;
t_log* CPULog;
sock_t* socketPlanificadorPadre;
t_cpu_padre configCPUPadre;
t_list* listaCPU;
pthread_mutex_t mutexListaCpus;
sem_t semCpuPadre;

//Firma de funciones
int conf_es_valida(t_config*);
int cargarArchivoDeConfiguracion();
void* ConectarAPlanificador();
int conectarCPUPadreAPlanificador();
t_pcb* escucharPlanificador();
void escucharYAtender(void);
int abrirArchivoYValidar(t_pcb*, sock_t*, sock_t*);
void limpiarRecursos();
char* informarAdminMemoriaComandoEscribir(int32_t, int32_t, char*, sock_t*);
char* informarAdminMemoriaComandoIniciar(char*, int32_t, sock_t*);
char* informarAdminMemoriaComandoFinalizar(int32_t, char*, sock_t*, sock_t*);
char* informarAdminMemoriaComandoLeer(int32_t , char* ,sock_t*);
char* informarEntradaSalida(t_pcb* , int32_t , char* , sock_t*);
int informarPlanificadorLiberacionCPU(t_pcb*, char*, sock_t*);
char* procesarInstruccion(char**, t_pcb*, char*,sock_t*,sock_t*,int32_t, char*);
void crearHilosCPU (void);
int hiloPadre();
void enviarCodigoOperacion(sock_t* , int32_t);
uint32_t deserializarEnteroSinSigno(sock_t*);
char* recibirMensaje(sock_t*);
int conectarCPUPadreAPlanificador();
char* serializarPCB(t_pcb*, uint32_t, char*);
void tituloInicial();
int32_t getPositionIfExists();
double initTimes(time_t*);
int calculateTimes(time_t*, double);
void actualizarTiempoAcumuladoEjecucion(int);
void iniciarCronTasks();
int32_t calculatePercent(uint32_t );
//void PorcentajeParaPlanificador();
char* depurarInstruccion(char*);
int fromSecondstoMicroSeconds(uint32_t);
void enviarPorcentaje();
FILE* abrirArchivo(t_pcb*);
#endif /* CPU_H_ */
