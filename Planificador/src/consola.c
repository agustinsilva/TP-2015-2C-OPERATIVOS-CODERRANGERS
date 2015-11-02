#include "planificador.h"


/*
 * @NAME: mostrarConsola
 * @DESC: Muestra la consola a ejecutarse
 */
void* mostrarConsola() {
		uint32_t* comando = malloc(sizeof(uint32_t));
		char path[255];
		uint32_t *pid = malloc(sizeof(uint32_t));
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
				scanf("%d",pid);
				finalizarProceso(pid);
				break;
			case 3:
				//Metodo que ejecuta el PS
				mostrarProcesos();
				break;
			case 4:
				//Metodo que ejecuta el Correr
				printf("Debera mostrar en pantalla del planificador un listado de ls CPUS actuales del sistema indicando para cada una su procentaje de uso del ultimo minuto");
//				pedirEstadoCpu();
				getchar();
				getchar();
				break;
			case 5:
				//Metodo que ejecuta el Correr
				printf("Debera matar a todos los procesos indicando la finalizacion");
				killThemAll();
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
	if (list_size(proc_bloqueados) > 0) {
		printf("Los programas en cola de bloqueados son:\n");
		for (index = 0; index < list_size(proc_bloqueados); ++index) {
			t_pcb *pcbListo = list_get(proc_bloqueados, index);
			printf("   PId: %d -- Nombre: %s -- Estado: "ANSI_COLOR_BLUE"%s\n"ANSI_COLOR_RESET, pcbListo->idProceso,
					pcbListo->path, convertirNumeroEnString(pcbListo->estadoProceso));
		}
	}
	if (list_size(proc_ejecutados) > 0) {
		printf("Los programas procesados son:\n");
		for (index = 0; index < list_size(proc_ejecutados); ++index) {
			t_pcb *pcb = list_get(proc_ejecutados, index);
			if (pcb->estadoProceso == 1) {
				printf("   PId: %d -- Nombre: %s -- Estado: "ANSI_COLOR_YELLOW"%s\n"ANSI_COLOR_RESET, pcb->idProceso,
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

/* Si el proceso esta en la cola de ready cambiarle el PC
 * Si el proceso estaba ejecutando, settearle un flag para cambiarlo en ready.
 * si es entrada y salida, lo mismo.
 */
void finalizarProceso(uint32_t *pid) {
	sem_wait(&mutex);
	int _pcbByPid(t_pcb *proc_ejecutado) {
		if (*pid == proc_ejecutado->idProceso)
			return 1;
		else
			return 0;
	}
	t_pcb *pcbFinListo = list_find(proc_listos, (void*) _pcbByPid);
	t_pcb *pcbFinEjecutados = list_find(proc_ejecutados, (void*) _pcbByPid);
	t_pcb *pcbFinBloqueados = list_find(proc_bloqueados, (void*) _pcbByPid);
	//Si el pcb esta en la cola de listos, le cambio el PC y listo
	if (pcbFinListo != NULL) {
		pcbFinListo->contadorPuntero = pcbFinListo->cantidadInstrucciones;
	}
	//Si el pcb esta en la cola ejecutados, le agrego un flag
	//Para que antes de ponerse en ready, se cambie el pc a la ultima instruccion.
	if (pcbFinEjecutados != NULL) {
		pcbFinEjecutados->flagFin = 1;
	}
	if (pcbFinBloqueados != NULL) {
		pcbFinBloqueados->flagFin = 1;
	}
	sem_post(&mutex);
}

void killThemAll(){
//	close(socketCpuPadre);
//	int index;
//	for (index = 0; index <= list_size(cpu_listos); ++index) {
//		t_hilosConectados *cpu = list_get(cpu_listos,index);
//		if(cpu->socketHilo != NULL)
//			close(cpu->socketHilo);
//	}
}

char* convertirNumeroEnString(uint32_t estado){
	if(estado==0)
		return "Espera";
	if(estado==1)
		return "Ejecucion";
	if(estado==2)
		return "Finalizado";
	if(estado==3)
		return "Bloqueado";
	else
		return "Flashiaste";
}

void leerComando(uint32_t* comando, char* mensaje) {
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
		pcb->contadorPuntero = 1;
		pcb->cantidadInstrucciones = cantidadInstrucciones;
		pcb->path = malloc(strlen(path));
		pcb->path = strdup(path); //TODO: revisar codigo maligno
		list_add(proc_listos,pcb);
		sem_post(&sincroproc);
		printf(ANSI_COLOR_GREEN "Se creo la pcb asociada y se introduce en la cola de ready a la espera de la cpu\n" ANSI_COLOR_RESET);
	}
	else{
		printf(ANSI_COLOR_RED "Se introdujo un path incorrecto.\n"ANSI_COLOR_RESET );
	}
	sem_post(&mutex);
}

void pedirEstadoCpu(){
	enviarCodigoOperacion(ESTADOCPUPADRE);
	//recibir los datos de los cpus y mostrarlos por consola
}

void enviarCodigoOperacion(int32_t entero){
	int32_t enviado = send(socketCpuPadre, &entero, sizeof(int32_t), 0);
	if(enviado!=sizeof(int32_t)){
		printf("No se envió correctamente la información entera\n");
		log_error(planificadorLog,"Error al enviar codigo de operacion a CPU Padre.","ERROR");
		return;
	}
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
