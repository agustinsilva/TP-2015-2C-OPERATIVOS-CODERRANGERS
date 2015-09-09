/***********PLANIFICADOR************/

#include "planificador.h"

int main(void)
{
	puts("Cargo archivo de configuracion de Planificador");
	cargarArchivoDeConfiguracion();
	mostrarConsola();
	iniciarServidor();

	puts("Fin de planificador");
	limpiarConfiguracion();
	return EXIT_SUCCESS;
}

/*
 * @NAME: mostrarConsola
 * @DESC: Muestra la consola a ejecutarse
 */
void* mostrarConsola() {
	int pid;

	pid = fork();

	if (pid<0) // Informamos Error
	{
		printf("ERROR IN FORK\n");
	} else if(pid) // Proceso hijo
	{
//		printf("HIJO: Soy el proceso hijo y mi pid es: %d\n", getpid());
//		printf("HIJO: mi padre tiene el pid: %d\n", getppid());

		int* comando = malloc(sizeof(int));
		while (1) {
			printf("-------------------- \n");
			printf("Bienvenido a la consola del Planificador \n");
			printf("Por favor seleccione un comando a realizar: \n");
			printf("-------------------- \n");
			printf("1 - Correr PATH\n");
			printf("2 - Finalizar PID \n");
			printf("3 - Estado de procesos \n");
			printf("4 - Cpu\n");
			printf("-------------------- \n");
			*comando = 0;
			leerComando(comando, "Debe ingresar un numero valido de comando\n");
			switch (*comando) {
			case 1:
				printf("-------------------- \n");
				printf("Por favor, ingrese el path del archivo que desea ejecutar:\n");
				//Metodo que ejecuta el Correr
				  getchar();
				  getchar();
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
				printf("Debera mostrar en pantalla el PID, nombre de programa y estado de cada proceso 'mProc'");
				  getchar();
				  getchar();
				break;
			case 4:
				//Metodo que ejecuta el Correr
				printf("Debera mostrar en pantalla del planificador un listado de ls CPUS actuales del sistema indicando para cada una su procentaje de uso del ultimo minuto");
				  getchar();
				  getchar();
				break;
			default:
				printf("Se selecciono un comando incorrecto.\n");
				break;
			}
		}
	}
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
