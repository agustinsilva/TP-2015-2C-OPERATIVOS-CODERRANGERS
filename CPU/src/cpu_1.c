/***********CPU************/

#include "cpu.h"

struct estructuraCPU {
  int ID_CPU;
  int ESTADO;
}

struct estructuraCPU CPU[configuracion->cantidadHilos];

int main(void) {

pthread_t hiloEscuchaPlanificador;

	puts("Comienzo de cpu");
	puts("Cargo archivo de configuracion de CPU");
	CPULog = log_create("CPULog", "CPU", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	crearHilosCPU(); //CREA LA CANTIDAD DE CPUs INDICADOS POR EL ARCHIVO DE CONFIGURACION
	ConectarAPlanificador();


	pthread_create(&hiloEscuchaPlanificador,NULL,(void*)escucharPlanificador,NULL); //HILO QUE ESCUCHA CONSTANTEMENTE LAS PETICIONES DEL PLANIFICADOR.

	puts("Fin de cpu \n");
	limpiarConfiguracion();
	log_destroy(CPULog);
	return EXIT_SUCCESS;
}

/**
 * CREA EL NUMERO DE HILOS QUE DICE EL ARCHIVO DE CONFIG
 * */
void crearHilosCPU (void)
{
  int cantidad=0;

  while (cantidad<configuracion->cantidadHilos)
    {
      strcpy(CPU[cantidad].ID_CPU,cantidad);
      strcpy(CPU[cantidad].ESTADO,0); /*Estado 0 disponible, Estado 1 ocupado*/
      printf("Se creo el hilo del CPU ID %i/n",CPU[cantidad].ID_CPU);
      cantidad++;
    }

}

void escucharPlanificador(void) /*Esta funcion deberia ponerse en un hilo para que escuche constantemente al Planificador por procesos entrantes*/
{
  while (1)
    {

      //--Declarar Variable mensaje y tipo de paquete que recibo.
      //--PREGUNTAR SI RECIBO ALGO DE PLANIFICADOR.
      //-- SI RECIBO ALGO-> Pregunto si hay algun HILO CPU sin ejectuar (ESTADO DISPONIBLE) --> EJECUTO crEANDO UN HILO EJECUCION CPU.
      //-- si no (else) Guardo el paquete en una LISTA DE PROCESOS ENCOLADOS.
    }
}
