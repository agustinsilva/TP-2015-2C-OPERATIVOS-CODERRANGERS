/***********CPU************/

#include "cpu.h"

int main(void) {
	CPULog = log_create("CPULog", "CPU", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	tituloInicial();
	pthread_mutex_init(&mutexListaCpus, NULL);
	sem_init(&semCpuPadre, 0, 0);
	sem_init(&semConexionPadre, 0, 0);
	sem_init(&semPorcentaje, 0, 1);
	iniciarCronTasks();
	hiloPadre();
	crearHilosCPU(); //CREA LA CANTIDAD DE CPUs INDICADOS POR EL ARCHIVO DE CONFIGURACION
	puts("Fin de cpu \n");
	limpiarRecursos();
	return EXIT_SUCCESS;
}

void crearHilosCPU() {
	sem_wait(&semConexionPadre);
	int rtaHilo = 0;
	int cantidad = 1;
	listaCPU = list_create();
	pthread_t threads[configuracion->cantidadHilos];
	uint32_t i;
	for (i = 0; i < configuracion->cantidadHilos; i++) {
		t_CPUsConectados *CPUsConectados = malloc(sizeof(t_CPUsConectados));
		rtaHilo = pthread_create(&threads[i], NULL, (void*) escucharYAtender,
				NULL);
		if (rtaHilo != 0) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n",
					rtaHilo);
			printf("Se cerrara el programa");
			exit(EXIT_FAILURE);
		}
		CPUsConectados->numeroCPU = i;
		CPUsConectados->idCPU = threads[i];
		CPUsConectados->porcentajeProcesado = 0;
		CPUsConectados->tiempoAcumuladoDeInstrucciones = 0;
		list_add(listaCPU, CPUsConectados);
		printf("Se creó nuevo hilo id: %u y se agregó a la lista.\n",
				threads[i]);
		//AGREGA EN UNA LISTACPU TODOS LOS HILOS QUE SE CREARON EN BASE A EL ARCHIVO DE CONFIG.
		cantidad++;
	}
	//printf("EL TAMAÑO DE LA LISTA ES %d\n", list_size(listaCPU));
	/*uint32_t k;
	 t_CPUsConectados *cpu;
	 for( k=0 ; k < list_size(listaCPU) ; k++){
	 cpu = list_get(listaCPU,k);
	 printf("iD de hilo: %u , en la pos: %u \n",cpu->idCPU,k);
	 }*/
	uint32_t j;
	t_CPUsConectados *posicion;
	for (j = 0; j <= list_size(listaCPU); j++) {
		posicion = list_get(listaCPU, j);
		pthread_join(posicion->idCPU, NULL);
	}
}

FILE* abrirArchivo(t_pcb* pcb){
	char* src = string_new();
	string_append(&src, "../Planificador/src/Codigos/");
	string_append(&src, pcb->path);
	FILE* entrada = fopen(src, "r");
	if (entrada == NULL) {
		log_error(CPULog, "No se pudo abrir el archivo de entrada. ", "ERROR");
	}
	log_info(CPULog, " [PID:%s] El archivo se abrió correctamente: %s, PC es %u\n",
			string_itoa(pcb->idProceso), pcb->path,pcb->contadorPuntero);

	free(src);
	return entrada;
}

int abrirArchivoYValidar(t_pcb* pcb, sock_t* socketPlanificador,
		sock_t* socketMemoria) {

	char* resultadosDeEjecuciones = string_new();
	int QUANTUMRESTANTE = configCPUPadre.quantum;
	char **lista;
	uint32_t numeroInstruccion = 1;
	char* instruccion = malloc(TAMINSTRUCCION);
	FILE* entrada = abrirArchivo(pcb);
	if(entrada == NULL){
		return 1;
	}
	while (pcb->contadorPuntero != numeroInstruccion) {
		fgets(instruccion, TAMINSTRUCCION+1, entrada);
		numeroInstruccion++;
	}

	if (configCPUPadre.tipoPlanificacion) { //RR
		int32_t cantInstruccionesEjecutadas = 0;
		while (QUANTUMRESTANTE > 0) {
			if (fgets(instruccion, TAMINSTRUCCION + 1, entrada) != NULL) {
				instruccion = depurarInstruccion(instruccion);
				lista = string_split(instruccion, " ");
				time_t *tiempo1 = malloc(sizeof(time_t));
				double tiempo_inicio_instruccion = initTimes(tiempo1);
				char* rta = procesarInstruccion(lista, pcb,
						resultadosDeEjecuciones, socketPlanificador,
						socketMemoria, cantInstruccionesEjecutadas,
						instruccion,entrada);
				int tiempo_ejecucion_instruccion = calculateTimes(tiempo1,
						tiempo_inicio_instruccion);
				pthread_mutex_lock(&mutexListaCpus);
				actualizarTiempoAcumuladoEjecucion(
						tiempo_ejecucion_instruccion);
				pthread_mutex_unlock(&mutexListaCpus);
				if (string_equals_ignore_case(rta, "FIN")) {
					break; //Termina la ejecucion porque:bloqueo de E/S, terminó el archivo
				} else {
					string_append(&resultadosDeEjecuciones, rta);
				}
				cantInstruccionesEjecutadas++;
				QUANTUMRESTANTE--;
			}
		}
		if (QUANTUMRESTANTE == 0) {
			log_info(CPULog, " [PID:%s] Finalizó quantum de ejecución.\n",
					string_itoa(pcb->idProceso));
			//Actualizamos el puntero del PCB
			pcb->contadorPuntero = pcb->contadorPuntero
					+ cantInstruccionesEjecutadas;
			//Enviamos el PCB al Planificador ya que terminó de ejecutar su Q.
			informarPlanificadorLiberacionCPU(pcb, resultadosDeEjecuciones,
					socketPlanificador);
		}
	} else { //FIFO
		int32_t cantInstruccionesEjecutadas = 0;
		while (fgets(instruccion, TAMINSTRUCCION + 1, entrada) != NULL) {
			instruccion = depurarInstruccion(instruccion);
			lista = string_split(instruccion, " ");
			time_t *tiempo1 = malloc(sizeof(time_t));
			double tiempo_inicio_instruccion = initTimes(tiempo1);
			char* rta = procesarInstruccion(lista, pcb, resultadosDeEjecuciones,
					socketPlanificador, socketMemoria,
					cantInstruccionesEjecutadas, instruccion,entrada);
			int tiempo_ejecucion_instruccion = calculateTimes(tiempo1,
					tiempo_inicio_instruccion);
			pthread_mutex_lock(&mutexListaCpus);
			actualizarTiempoAcumuladoEjecucion(tiempo_ejecucion_instruccion);
			pthread_mutex_unlock(&mutexListaCpus);
			if (string_equals_ignore_case(rta, "FIN")) {
				break; //Termina la ejecucion porque: bloqueo de E/S; terminó el archivo
			} else {
				string_append(&resultadosDeEjecuciones, rta);
			}
			cantInstruccionesEjecutadas++;
		}
	}
	fclose(entrada);
	free(instruccion);
	//free(src);

	log_info(CPULog, " [PID:%s] Se cerró el archivo.\n",
			string_itoa(pcb->idProceso));
	return 0;
}

/** Funcion que:
 * 		Informa al Planificador la creacion de un hilo
 *		Queda a la espera de recibir instrucciones del Planificador
 */
void escucharYAtender() {
	t_pcb* pcb;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	sock_t* socketClientePlanificador = create_client_socket(
			configuracion->ipPlanificador, configuracion->puertoPlanificador);
	int32_t conexionPlanificador = connect_to_server(socketClientePlanificador);

	if (conexionPlanificador != 0) {
		log_error(CPULog, "Error al conectar CPU a Planificador\n", "ERROR");
	}
	log_info(CPULog,
			"Se conectó planificador al cpu correctamente. Socket id: %u\n",
			socketClientePlanificador->fd);

	//Envia aviso al Planificador de que se creó un nuevo hilo CPU.
	enviarCodigoOperacion(socketClientePlanificador, NUEVO_HILO);

	//Conexion del Hilo CPU con Memoria
	sock_t* clientSocketAdmin = create_client_socket(configuracion->ipMemoria,
			configuracion->puertoMemoria);
	int32_t conexionAdminMemoria = connect_to_server(clientSocketAdmin);

	if (conexionAdminMemoria != 0) {
		perror("Error al conectar socket");
		log_error(CPULog, "Error al conectar CPU a Administrador de Memoria. ",
				"ERROR");
		clean_socket(clientSocketAdmin);
		return;
	}
	log_info(CPULog, "Se conectó a Memoria correctamente. Socket id: %u\n",
			clientSocketAdmin->fd);

	while (1) {
		pthread_mutex_lock(&mutex);
		pcb = escucharPlanificador(socketClientePlanificador);
		if (pcb == NULL) {
			free(pcb);
			break;
		} else {
			abrirArchivoYValidar(pcb, socketClientePlanificador,
					clientSocketAdmin);
			free(pcb->path);
			free(pcb);
		}
		pthread_mutex_unlock(&mutex);
	}
}

int hiloPadre() {
	pthread_t hiloCpuPadre;
	int rtaHiloPadre;
	rtaHiloPadre = pthread_create(&hiloCpuPadre, NULL,
			(void*) conectarCPUPadreAPlanificador, NULL);
	if (rtaHiloPadre) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n",
				rtaHiloPadre);
		printf("Se cerrara el programa");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

char* procesarInstruccion(char **lista, t_pcb *pcb,
		char* resultadosDeEjecuciones, sock_t* socketPlanificador,
		sock_t* socketMemoria, int32_t cantInstruccionesEjecutadas,
		char *instruccion, FILE* entrada) {
	char* rta;
	if (string_equals_ignore_case(lista[0], "iniciar")) {
		log_info(CPULog, " [PID:%s] Instruccion: iniciar",
								string_itoa(pcb->idProceso));
		rta = informarAdminMemoriaComandoIniciar(lista[1], pcb->idProceso,
				socketMemoria);
		usleep(fromSecondstoMicroSeconds(configuracion->retardo));
	} else if (string_equals_ignore_case(lista[0], "finalizar")) {
		log_info(CPULog, " [PID:%s] Instruccion: finalizar",
				string_itoa(pcb->idProceso));
		//Informar al AdminMemoria que finalice el proceso
		rta = informarAdminMemoriaComandoFinalizar(pcb->idProceso,
				resultadosDeEjecuciones, socketPlanificador, socketMemoria);
		usleep(fromSecondstoMicroSeconds(configuracion->retardo));
	} else if (string_equals_ignore_case(lista[0], "leer")) {
		log_info(CPULog, " [PID:%s] Instruccion: leer pág: %s",
				string_itoa(pcb->idProceso), lista[1]);
		//lista[1] contiene el nro de pagina
		rta = informarAdminMemoriaComandoLeer(pcb->idProceso, lista[1],
				socketMemoria);
		usleep(fromSecondstoMicroSeconds(configuracion->retardo));
		if(string_equals_ignore_case(rta, "ABORTAR")){
			abortarPCB(pcb,entrada);
		}
	} else if (string_equals_ignore_case(lista[0], "escribir")) {
		char* textoEscribir = string_substring(instruccion, string_length("escribir k"),string_length(instruccion));
		int32_t tamanioTexto = strlen(textoEscribir);
		textoEscribir[tamanioTexto] = '\0';
		int32_t numeroPagina = (int32_t) strtol(lista[1], NULL, 10);
		log_info(CPULog, " [PID:%s] Instruccion: escribir en página %s: %s",
				string_itoa(pcb->idProceso), string_itoa(numeroPagina),
				textoEscribir);
		//lista[1] Contiene numero de página, lista[2] contiene el texto que se quiere a escribir en esa pagina.
		rta = informarAdminMemoriaComandoEscribir(pcb->idProceso, numeroPagina,
				textoEscribir, socketMemoria);
		usleep(fromSecondstoMicroSeconds(configuracion->retardo));
		free(textoEscribir);
		if(string_equals_ignore_case(rta, "ABORTAR")){
			abortarPCB(pcb,entrada);
		}
	} else if (string_equals_ignore_case(lista[0], "entrada-salida")) {
		int32_t tiempoDeEjec = (int32_t) strtol(lista[1], NULL, 10);
		log_info(CPULog, "[PID:%s] Instruccion: entrada-salida de tiempo %s.",
				string_itoa(pcb->idProceso), string_itoa(tiempoDeEjec));
		//lista[1] Contiene el tiempo que se debe bloquear.
		usleep(fromSecondstoMicroSeconds(configuracion->retardo));
		//actualizamos el puntero del pcb
		pcb->contadorPuntero = pcb->contadorPuntero
				+ cantInstruccionesEjecutadas + 1;
		rta = informarEntradaSalida(pcb, tiempoDeEjec, resultadosDeEjecuciones,
				socketPlanificador);
	} else {
		log_warning(CPULog, " [PID:%s] Instruccion: comando no interpretado",
				string_itoa(pcb->idProceso));
		rta = "Comando no interpretado.\n";
	}
	return rta;
}

/* Mueve el puntero de instruccion del PCB
 * a la última instrucción.
*/
void abortarPCB(t_pcb* pcb, FILE* entrada){
	uint32_t numeroInstruccion = pcb->contadorPuntero;
	char* instruccion = malloc(TAMINSTRUCCION);
	while (pcb->cantidadInstrucciones != numeroInstruccion) {
		fgets(instruccion, TAMINSTRUCCION+1, entrada);
		numeroInstruccion++;
	}
	pcb->contadorPuntero=(pcb->cantidadInstrucciones -1);
	free(instruccion);
	//log_info(CPULog, "[PID:%s]ABORTO - Debería ir a finalizar, el PC ahora es %u\n",string_itoa(pcb->idProceso),pcb->contadorPuntero);
}

/***********-----------GESTION-----------**************/

void listaCpuDestroyer(t_CPUsConectados* cpu) {
	free(cpu);
}

void limpiarRecursos() {
	list_destroy_and_destroy_elements(listaCPU, (void*) listaCpuDestroyer);
	config_destroy(fd_configuracion);
	log_destroy(CPULog);
	free(configuracion);
}

void tituloInicial() {
	int32_t i;
	printf("\n\t\t");
	for (i = 226; i < 232; i++) {
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
	}
	printf("\e[48;5;231m" BOLD "\e[30mCPU" RESET_NON_BOLD "\e[0m");
	for (i = 231; i > 225; i--) {
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
		printf("\e[48;5;%dm \e[0m" "", i);
	}

	printf("\n\n");
}

char* depurarInstruccion(char* instruccion) {
	instruccion[string_length(instruccion) - 2] = '\0';
	return instruccion;
}

int fromSecondstoMicroSeconds(uint32_t seconds) {
	return seconds * 1000000;
}
