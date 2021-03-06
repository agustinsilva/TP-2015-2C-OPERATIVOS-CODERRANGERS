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
#include <signal.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/time.h>

// Constantes
#define RESET "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define HIDDEN "\e[8m"
#define BOLD "\e[1m"
#define UNDERLINED "\e[4m"
#define RESET_NON_BOLD "\e[21m"
#define RESET_NON_UL "\e[24m"
#define BLINK "\e[5m"
#define RESET_NON_BLINK "\e[25m"

#define codigo_iniciar 1
#define codigo_finalizar 2
#define codigo_leer 3
#define codigo_escribir 4
#define codigo_morir 5
#define pedido_exitoso 1
#define pedido_error 0
#define pedido_no_memoria -1
#define pedido_aborto 99
#define swap_in -2
#define marcos_insuficientes -1
#define marcos_no_libres -2
#define marco_inhabilitado -3
#define FIFO "FIFO"
#define CLOCKM "CLOCK-M"
#define LRU "LRU"
#define REINIT -1
#define tabla_paginas 1
#define memoria_principal 2
#define dummy -5
#define marco_victima -4
#define stat_TLB -5

//Estructuras
typedef struct
{
	uint32_t puerto_escucha;
	char* ip_swap;
	int32_t puerto_swap;
	int32_t maximo_marcos_por_proceso;
	int32_t cantidad_marcos;
	int32_t tamanio_marco;
	int32_t entradas_tlb;
	int32_t tlb_habilitada;
	double retardo_memoria;
	char* algoritmo_reemplazo;
} t_Memoria_Config;

typedef struct
{
	sock_t* cpuSocket;
	sock_t* swapSocket;
} t_HiloCPU;

typedef struct
{
	int32_t	marco;
	int32_t	pagina;
	int32_t idProc;
} t_TLB;

typedef struct
{
	int32_t	marco;
	bool ocupado;
	char* contenido;
} t_MP;

typedef struct
{
	int32_t	idProc;
	int32_t	frame;
	int32_t	nroPag;
	int32_t loadedTime; //para FIFO
	int32_t usedTime; // para LRU
	bool present;  // para MP
	bool modified; // para CM
	bool accessed; // para CM
} t_TP;

typedef struct
{
	int32_t encontro;
	int32_t longitud;
	char* contenido;
}t_LecturaSwap;

typedef struct {
	int32_t idProc;
	int32_t pagsTotales;
	int32_t pageFaults;
	int32_t miss;
	int32_t hit;
}t_Stats;

typedef struct {
	int32_t idProc;
	t_list* marcos; /* de t_Orden*/
}t_Marcos;

typedef struct{
	int32_t marco;
	int32_t orden;
	int32_t nroPag;
	bool puntero; // para CM
}t_Orden;


//Variables globales
t_Memoria_Config* configuracion;
t_config * fd_configuracion;
t_log* MemoriaLog;
t_list* TLB; /* de t_TLB */
t_list* memoriaPrincipal; /* de t_MP */
t_list* tablasDePaginas; /* de t_TP */
sock_t* clientSocketSwap;
t_list* CPUsConectados; /* de sock_t */
t_list* estadisticas;
t_list* ordenMarcos; /* de t_Marcos */

pthread_mutex_t sem_TLB;
pthread_mutex_t sem_TP;
pthread_mutex_t sem_MP;
pthread_mutex_t sem_swap;
pthread_mutex_t sem_stats;
pthread_mutex_t sem_order;

//Firma de funciones

/* Principales */
char* recibirMensaje(sock_t* socket);
int32_t enviarMensaje(sock_t* socket, char* mensaje);
int32_t conf_es_valida(t_config * configuracion);
int cargarArchivoDeConfiguracion();
void limpiarConfiguracion();
int32_t hiloEjecucionCPU(t_HiloCPU* );
void setUp();
void limpiarMemoriaPrincipal();
void limpiarTLB();
void TLBDestroyer(t_TLB* );
void saludoInicial();
void initializeMutex();
void limpiarCPUs();
void limpiarEstadisticas();
void iniciarCronTasks();
void limpiarOrdenMarcos();
void marcosDestroyer(t_Marcos* );
void ordenDestroyer(t_Orden* );

/* de AtencionPedidosCPU */
int32_t recibirCodigoOperacion(sock_t*);
void iniciar(sock_t* , sock_t* );
void finalizar(sock_t* , sock_t* );
void lectura(sock_t* , sock_t* );
void escritura(sock_t* , sock_t* );
t_LecturaSwap* pedirPagina(sock_t* , int32_t , int32_t );
void enviarEnteros(sock_t* , int32_t );
void enviarStrings(sock_t* , char* , int32_t );
void enviarContenidoPagina(sock_t* , t_LecturaSwap* );

/* de GestionMemoria */
int32_t crearTablaDePaginas(int32_t , int32_t );
void eliminarTablaDePaginas(int32_t );
void procesoDestroyer(t_TP* );
void limpiarRecursos();
t_TLB* actualizarTLB(int32_t, int32_t , int32_t );
void eliminarDeTLBPorPID(int32_t );
void eliminarDeTLBPorMarco(int32_t );
int32_t swapIN(sock_t* , sock_t* , int32_t , int32_t , int32_t);
void manejarMemoriaPrincipalLectura(t_MP* , sock_t* );
void manejarMemoriaPrincipalEscritura(t_MP* , sock_t* , char* , int32_t , int32_t);
int32_t calcularCantPaginasEnMP(int32_t );
t_MP* actualizarMP(int32_t , int32_t , int32_t , t_LecturaSwap* );
int32_t getRandomFrameVacio();
int32_t reemplazarMP(int32_t , char* );
int32_t reemplazarFIFO(t_list*);
int32_t reemplazarCLOCKM(t_list*,int32_t);
int32_t reemplazarLRU(t_list*);
int32_t setLoadedTimeForProc(int32_t);
int32_t getMinLoadedTime(t_list* );
int32_t getMaxUsedTime(t_list* );
t_list* getTablaDePaginasPresentes(int32_t );
void vaciarMarcosOcupados(int32_t );
bool escribirEnSwap(t_TP* , sock_t* );
void retardo(double, int32_t, int32_t, int32_t, int32_t);
void llenarDeNulos(char* , int32_t ,int32_t);
void abortarProceso(int32_t);
t_Orden* crearElementoOrden(int32_t , int32_t , int32_t , bool , t_list* );
t_Marcos* crearElementoOrdenMarcos(int32_t , int32_t , int32_t , int32_t , bool );
void ordenarPorCargaMarcos(t_list*);
void adelantarPuntero(t_list* );
void eliminarOrdenMarcos(int32_t);
int32_t doSwap(int32_t , int32_t , int32_t , int32_t , sock_t* , sock_t* );
void avanzarTiempoLRU(int32_t , int32_t);
void avanzarTiempoFIFO(int32_t );
void avanzarTiempo(int32_t , int32_t );



/* de Signals.c */
void signalHandler();
void finalizacion();
void MPFush();
void MPDump();
void TLBFlush();
void doTLBFlush();
void doMPFlush(sock_t*);

void escribirPagsModificadas(sock_t*);
void actualizarTablaDePaginas();
void vaciarMemoria();
void printearTabla();
void statsPerMinute();

/* de busquedas.c*/
t_TLB* buscarEnTLB(int32_t , int32_t );
t_MP* buscarEnMemoriaPrincipal(int32_t);
t_TP* buscarEntradaEnTablaDePaginas(int32_t , int32_t );
t_TP* buscarEnTablaDePaginasByMarco(int32_t );
int32_t buscarMarcoEnTablaDePaginas(int32_t, int32_t);
t_Stats* buscarEstadisticaPorProceso(int32_t );


#endif /* ADMINMEMORIA_H_ */
