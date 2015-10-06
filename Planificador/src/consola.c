#include "planificador.h"


/*
 * @NAME: mostrarConsola
 * @DESC: Muestra la consola a ejecutarse
 */
void* mostrarConsola() {
		int* comando = malloc(sizeof(int));
		char path[255];
		while (1) {
			printf("-------------------- \n");
			printf("Bienvenido a la consola del Planificador \n");
			printf("Por favor seleccione un comando a realizar: \n");
			printf("-------------------- \n");
			printf("1 - Correr PATH\n");
			printf("2 - Finalizar PID \n");
			printf("3 - Estado de procesos \n");
			printf("4 - Cpu\n");
			printf("5 - Kill Them All\n");
			printf("-------------------- \n");
			*comando = 0;
			leerComando(comando, "Debe ingresar un numero valido de comando\n");
			switch (*comando) {
			case 1:
				printf("-------------------- \n");
				printf("Por favor, ingrese el path del archivo que desea ejecutar:\n");
				//Metodo que ejecuta el Correr
				scanf("%s",path);
				encolar(path);
				break;
			case 2:
				printf("-------------------- \n");
				printf("Indique el PID del proceso que quiere finalizar: \n");
				//Metodo que ejecuta el finalizar proceso
				  getchar();
				  getchar();
				break;
			case 3:
				//Metodo que ejecuta el PS
				mostrarProcesos();
				break;
			case 4:
				//Metodo que ejecuta el Correr
				printf("Debera mostrar en pantalla del planificador un listado de ls CPUS actuales del sistema indicando para cada una su procentaje de uso del ultimo minuto");
				  getchar();
				  getchar();
				break;
			case 5:
				//Metodo que ejecuta el Correr
				printf("Debera matar a todos los procesos indicando la finalizacion");
				  getchar();
				  getchar();
				break;
			default:
				printf("Se selecciono un comando incorrecto.\n");
				break;
		}
	}
}

void mostrarProcesos() {
	int index;
	sem_wait(&mutex);
	if (list_size(proc_listos) > 0) {
		printf("Los programas en cola de ready son:\n");
		for (index = 0; index < list_size(proc_listos); ++index) {
			t_pcb *pcbListo = list_get(proc_listos, index);
			printf("    PId: %d -- Nombre: %s -- Estado: "ANSI_COLOR_GREEN"%s\n"ANSI_COLOR_RESET, pcbListo->idProceso,
					pcbListo->path, convertirNumeroEnString(pcbListo->estadoProceso));
		}
	} else
		printf("No hay programas en espera de ejecucion\n");

	if (list_size(proc_ejecutados) > 0) {

		printf("Los programas procesados son:\n");
		for (index = 0; index < list_size(proc_ejecutados); ++index) {
			t_pcb *pcb = list_get(proc_ejecutados, index);
			if (pcb->estadoProceso == 1) {
				printf("    PId: %d -- Nombre: %s -- Estado: "ANSI_COLOR_YELLOW"%s\n"ANSI_COLOR_RESET, pcb->idProceso,
						pcb->path, convertirNumeroEnString(pcb->estadoProceso));
			}

		}
		for (index = 0; index < list_size(proc_ejecutados); ++index) {
			t_pcb *pcb = list_get(proc_ejecutados, index);
			if (pcb->estadoProceso == 2) {
				printf("   PId: %d -- Nombre: %s -- Estado: "ANSI_COLOR_RED"%s\n"ANSI_COLOR_RESET, pcb->idProceso, pcb->path, convertirNumeroEnString(pcb->estadoProceso));
			}

		}
	} else
		printf("No hay programas finalizados\n");
	printf("Presione enter para volver al menu...\n");
	sem_post(&mutex);
	getchar();
	getchar();
}

char* convertirNumeroEnString(uint32_t estado){
	if(estado==0)
		return "Espera";
	if(estado==1)
		return "Ejecucion";
	else
		return "Finalizado";
}

void leerComando(int* comando, char* mensaje) {
	int c;
	if (!scanf("%d", comando)) {
		printf("%s", mensaje);
		while ((c = fgetc(stdin)) != EOF && c != '\n') {
			fflush(stdin);
		}
	}
}

void encolar(char* path) {
	int cantidadInstrucciones = contarInstrucciones(path);
	sem_wait(&mutex);
	if(cantidadInstrucciones){ //Si se abre el archivo, creo pcb
		t_pcb *pcb = malloc(sizeof(t_pcb));
		pcb->idProceso = contadorProceso;
		contadorProceso++;
		pcb->estadoProceso = 0; //0-Espera 1-Ejecucion 2-Finalizado
		pcb->contadorPuntero = 0;
		pcb->cantidadInstrucciones = cantidadInstrucciones;
		pcb->path = malloc(strlen(path));
		pcb->path = strdup(path);
		list_add(proc_listos,pcb);
		sem_post(&sincroproc);
		printf(ANSI_COLOR_GREEN "Se creo la pcb asociada y se introduce en la cola de ready a la espera de la cpu\n" ANSI_COLOR_RESET);
	}
	else{
		printf(ANSI_COLOR_RED "Se introdujo un path incorrecto.\n"ANSI_COLOR_RESET );
	}
	sem_post(&mutex);
}

int contarInstrucciones(char* path) {
	int ch;
	int cantidad_lineas = 1;
	char* src = string_new();
	string_append(&src, "src/Codigos/");
	string_append(&src, path);
	FILE* fp = fopen(src, "r");
	if (fp == NULL) {
		return 0;
	}
	do {
		ch = fgetc(fp);
		if (ch == '\n')
			cantidad_lineas++;
	} while (ch != EOF);
	fclose(fp);
	return cantidad_lineas;
}
