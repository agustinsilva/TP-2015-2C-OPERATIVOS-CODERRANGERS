/***********CPU************/

#include "cpu.h"

int main(void)
{
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

int abrirArchivoYValidar(char* path){
	char **lista;
	int instrucciones=0;
	//	int instructionPointer=0; Cuando se ejecuta finalizar tengo que ir a la ultima insturccion para eso cuento todas?
	char instruccion[TAMINSTRUCCION];
	FILE* entrada;

	if((entrada=fopen(path,"r"))==NULL){
		log_error(CPULog,"No se pudo abrir el archivo de entrada. \n","ERROR");
		return -1;
	}

	printf("Archivo abierto \n");
	log_info(CPULog,"El archivo se abrio correctamente: %s \n",path,"INFO");

	while (fgets(instruccion,TAMINSTRUCCION, entrada) != NULL) {
		lista = string_split(instruccion," ");

		if (string_equals_ignore_case(lista[0], "iniciar")){
			puts("Instruccion: iniciar\n");

			//SE CONECTA A MEMORIA//
			//lista[1] contiene la cantidad de paginas a pedir al AdminMemoria
			informarAdminMemoriaComandoIniciar(lista[1]);

			sleep(configuracion->retardo);
			//instructionPointer++; VER SI VA
		}else if(string_equals_ignore_case(lista[0], "finalizar")){
			puts("Instruccion: finalizar\n");

			//SE CONECTA A MEMORIA//
			//Informar al AdminMemoria que finalice el proceso
			informarAdminMemoriaComandoFinalizar(path);

			sleep(configuracion->retardo);
		}else{
			printf("Comando no interpretado: %s",lista[0]);
		}
	}

	fclose(entrada);
	puts("Se cerró el archivo\n");

	return 0;
}

/** Funcion que:
 * 		Informa al Planificador la creacion de un hilo
 *		Queda a la espera de recibir instrucciones del Planificador
 */
void escucharYAtender(){
	t_pcb pcb;
	pcb = escucharPlanificador();
	char* path = pcb.path;
	printf("El path recibido es: %s \n",path);
	abrirArchivoYValidar(path);
}

