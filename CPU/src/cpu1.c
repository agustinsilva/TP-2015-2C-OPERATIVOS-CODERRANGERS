/***********CPU************/

#include "cpu.h"
#include <pthread.h>

struct estructuraCPU {
  int ID_CPU;
  int ESTADO;
  
}

struct estructuraCPU CPU[configuracion->cantidadHilos];

int main(void) {

  pthread_t hiloEscuchaPlanificador; 

	puts("Comienzo de cpu\n");
	puts("Cargo archivo de configuracion de CPU\n");
	CPULog = log_create("CPULog", "CPU", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	ConectarAPlanificador();
	crearCPU (); //CREA LAS ESTRUCTURAS DE LOS CPUs INDICADOS POR EL ARCHIVO DE CONFIGURACION Y LAS INICIALIZA EN ESTADO DISPONIBLE.
	pthread_create(&hiloEscuchaPlanificador,NULL,(void*)escucharPlanificador,NULL); //HILO QUE ESCUCHA CONSTANTEMENTE LAS PETICIONES DEL PLANIFICADOR.
	
	puts("Fin de cpu \n");
	limpiarConfiguracion();
	log_destroy(CPULog);
	return EXIT_SUCCESS;
}


void crearCPU (void) /*CREA LAS ESTRUCTURAS DE CPU QUE DICE EL ARCHIVO DE CONFIG*/
{

  int cantidad=0;

  while (cantidad<configuracion->cantidadHilos)
    {
    
      strcpy(CPU[cantidad].ID_CPU,cantidad);
      strcpy(CPU[cantidad].ESTADO,0); /*Estado 0 disponible, Estado 1 ocupado*/
          
      printf("Se creo la estructura del CPU ID %i/n",CPU[cantidad].ID_CPU);
      cantidad++;
    }

}

void escucharPlanificador(void) /*Esta funcion deberia ponerse en un hilo para que escuche constantemente al Planificador por procesos entrantes*/
{
  while (1) 
    {

      //listen_connections();
      //--Declarar Variable mensaje y tipo de paquete que recibo.
      //--PREGUNTAR SI RECIBO ALGO DE PLANIFICADOR.
     
      //-- SI RECIBO ALGO-> Pregunto si hay algun HILO CPU sin ejectuar (ESTADO DISPONIBLE) --> EJECUTO crEANDO UN HILO EJECUCION CPU.
      pthread_t HiloCPU1;
      pthread_create(&HiloCPU1,NULL,(void*)ejecutarPrograma,NULL); 
      //-- si no (else) Guardo el paquete en una LISTA DE PROCESOS ENCOLADOS.
    }
}
