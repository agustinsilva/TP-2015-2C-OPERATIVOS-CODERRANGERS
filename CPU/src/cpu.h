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

//Constantes
#define PAQUETE 1024
#define TAMINSTRUCCION 80
#define INICIAR 1
#define FINALIZAR 2
#define RESPUESTA_PLANIFICADOR 2
#define RESPUESTA_PLANIFICADOR_FIN_EJECUCION 3
#define LEER 3
#define ESCRIBIR 4
#define ANORMAL 5
#define NUEVO_HILO 1
#define PEDIDO_ERROR 0

//Variables globales
t_configuracion* configuracion;
t_config * fd_configuracion;
t_log* CPULog;
sock_t* socketAdminMemoria;
sock_t* socketPlanificador;

//Firma de funciones
int conf_es_valida(t_config* configuracion);
int cargarArchivoDeConfiguracion();
void* ConectarAPlanificador();
t_pcb* escucharPlanificador();
void escucharYAtender();
void limpiarConfiguracion();
int informarAdminMemoriaComandoIniciar(char* cantidadPaginas, int32_t pid);
int informarAdminMemoriaComandoFinalizar(int32_t pid);
int informarAdminMemoriaComandoLeer(int32_t pid, char* numeroPagina);
void crearHilosCPU (void);
void enviarCodigoOperacion(sock_t* socket, int32_t entero);
uint32_t deserializarEnteroSinSigno(sock_t* socket);
#endif /* CPU_H_ */
