/***********CPU************/

#include "cpu.h"

struct estructuraCPU {
  unsigned long int ID_CPU;
  int ESTADO;
};

//struct estructuraCPU CPU[configuracion->cantidadHilos];

int main(void) {

	puts("Comienzo de cpu");
	puts("Cargo archivo de configuracion de CPU");
	CPULog = log_create("CPULog", "CPU", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	//ConectarAPlanificador();
	crearHilosCPU(); //CREA LA CANTIDAD DE CPUs INDICADOS POR EL ARCHIVO DE CONFIGURACION

	puts("Fin de cpu \n");
	limpiarConfiguracion();
	log_destroy(CPULog);
	return EXIT_SUCCESS;
}

/**
 * CREA EL NUMERO DE HILOS QUE DICE EL ARCHIVO DE CONFIG
 * 		para checkpoint 2 creamos solo 1 hilo
 * */
void crearHilosCPU (void)
{
	int cantidad=0;
	int rtaHilo = 0;
	pthread_t hiloCpu; //id de cpu

	rtaHilo = pthread_create(&hiloCpu,NULL,(void*)escucharYAtender,NULL);
	if(rtaHilo)
	{
		fprintf(stderr,"Error - pthread_create() return code: %d\n",rtaHilo);
		printf("Se cerrara el programa");
		exit(EXIT_FAILURE);
	}
	pthread_join(hiloCpu, NULL);
	//  while (cantidad<configuracion->cantidadHilos)
	//    {
	//      strcpy(CPU[cantidad].ID_CPU,cantidad);
	//      strcpy(CPU[cantidad].ESTADO,0); /*Estado 0 disponible, Estado 1 ocupado*/
	//      printf("Se creo el hilo del CPU ID %i/n",CPU[cantidad].ID_CPU);
	//      cantidad++;
	//    }

}

void escucharYAtender(){
	t_pcb pcb;
	//pcb = escucharPlanificador();

	pcb.path = "src/prueba.txt"; //PARA PROBAR
	char* path = pcb.path;
	printf("El path recibido es: %s \n",path);
	abrirArchivoYValidar(path);
}

int abrirArchivoYValidar(char* path){
	FILE* entrada;
	if((entrada=fopen(path,"r"))==NULL){
		log_error(CPULog,"No se pudo abrir el archivo de entrada. \n","ERROR");
		return -1;
	}
	printf("Archivo abierto \n");

	//COMIENZA A LEER CADA LINEA DEL ARCHIVO

	return 0;
}
