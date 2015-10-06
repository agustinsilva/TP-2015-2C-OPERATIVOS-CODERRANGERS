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
#define codigo_iniciar 1
#define codigo_finalizar 2
#define codigo_leer 3
#define codigo_escribir 4
#define codigo_morir 5
#define pedido_exitoso 1
#define pedido_error 0
#define pedido_no_memoria -1
#define swap_in -2

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
	int32_t retardo_memoria;
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
	int32_t	offset;
	bool present;
	bool modified;
	bool accessed;
} t_TP;

typedef struct
{
	int32_t encontro;
	int32_t longitud;
	char* contenido;
}t_LecturaSwap;

//Variables globales
t_Memoria_Config* configuracion;
t_config * fd_configuracion;
t_log* MemoriaLog;
t_list* TLB; /* de t_TLB */
t_list* memoriaPrincipal; /* de t_MP */
t_list* tablasDePaginas; /* de t_TP */

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
int32_t eliminarTablaDePaginas(int32_t );
void procesoDestroyer(t_TP* );
int32_t getPagina();
int32_t getFrame();
bool hayEspacio();
void limpiarRecursos();
void actualizarTLB(int32_t, int32_t , int32_t );
void swapIN(sock_t* , sock_t* , int32_t , int32_t );
void manejarMemoriaPrincipal(t_MP* , sock_t* );
t_TLB* buscarEnTLB(int32_t , int32_t );
t_MP* buscarEnMemoriaPrincipal(int32_t);
int32_t buscarMarcoEnTablaDePaginas(int32_t, int32_t);
void manejarMemoriaPrincipal(t_MP* , sock_t* );

#endif /* ADMINMEMORIA_H_ */
