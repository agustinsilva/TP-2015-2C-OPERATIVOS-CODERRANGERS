/***********CPcbU************/

#include "cpu.h"

int main(void)
{
	CPULog = log_create("CPULog", "CPU", true, LOG_LEVEL_INFO);
	cargarArchivoDeConfiguracion();
	hiloPadre();
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
void crearHilosCPU()
{
	int rtaHilo = 0;
	pthread_t hiloCpu;
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

int abrirArchivoYValidar(t_pcb* pcb){
	int QUANTUMRESTANTE = configCPUPadre.quantum;
	char **lista;
	uint32_t numeroInstruccion=0;
	char instruccion[TAMINSTRUCCION];
	char* src = string_new();
	string_append(&src, "../Planificador/src/Codigos/");
	string_append(&src, pcb->path);
	FILE* entrada = fopen(src, "r");
	if(entrada==NULL){
		log_error(CPULog,"No se pudo abrir el archivo de entrada. ","ERROR");
		return -1;
	}
	log_info(CPULog,"El archivo se abrio correctamente: %s ",pcb->path,"INFO");

	//Conectar con Admin de Memoria
	sock_t* clientSocketAdmin = create_client_socket(configuracion->ipMemoria,configuracion->puertoMemoria);
	int32_t conexionAdminMemoria = connect_to_server(clientSocketAdmin);
	if (conexionAdminMemoria != 0) {
		perror("Error al conectar socket");
		log_error(CPULog,"Error al conectar CPU a Administrador de Memoria. ","ERROR");
		exit(EXIT_FAILURE);
	}
	socketAdminMemoria = clientSocketAdmin;
	while(pcb->contadorPuntero != numeroInstruccion){
		fgets(instruccion,TAMINSTRUCCION+1, entrada); //TOMA LA LINEA E INCREMENTA Y SIGUE CON LA SIGUIENTE.
		numeroInstruccion ++;
	}

	if (configCPUPadre.tipoPlanificacion==1) {//RR
		printf("Es RR\n");
		int32_t cantInstruccionesEjecutadas = 0;
		while (QUANTUMRESTANTE > 0) {
			if(fgets(instruccion,TAMINSTRUCCION+1, entrada) != NULL) {
				lista = string_split(instruccion," ");
				procesarInstruccion(lista,pcb->idProceso);
				cantInstruccionesEjecutadas++;
				QUANTUMRESTANTE--;
			}else{
				break;
			}
		}
		if(QUANTUMRESTANTE == 0){
			log_info(CPULog," [PID:%s] Finalizó quantum de ejecución.\n",string_itoa(pcb->idProceso));
			//actualizamos el puntero del pcb
			pcb->contadorPuntero = pcb->contadorPuntero + cantInstruccionesEjecutadas;
			//enviamos el pcb al planificador ya que terminó de ejecutar su quantum
			informarPlanificadorLiberacionCPU(pcb);
		}
	}else{//FIFO
		printf("Es FIFO\n");
		while(fgets(instruccion,TAMINSTRUCCION+1, entrada) != NULL) {
			lista = string_split(instruccion," ");
			procesarInstruccion(lista,pcb->idProceso);
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
void escucharYAtender()
{
	sock_t* socketClientePlanificador = create_client_socket(configuracion->ipPlanificador,configuracion->puertoPlanificador);
	socketPlanificador = socketClientePlanificador;
	int32_t conexionPlanificador = connect_to_server(socketClientePlanificador);
	if (conexionPlanificador != 0)
	{
		log_error(CPULog,"Error al conectar CPU a Planificador","ERROR");
	}
	log_info(CPULog,"Se conectó planificador al cpu correctamente.");
	//Envia aviso al Plani de que se creó un nuevo hilo cpu.
	enviarCodigoOperacion(socketPlanificador,NUEVO_HILO);
	t_pcb* pcb;
	while(1)
	{
		pcb = escucharPlanificador();
		printf("El path recibido es: %s \n",pcb->path);
		abrirArchivoYValidar(pcb);
		free(pcb->path);
		free(pcb);
	}
}

int hiloPadre(){
	pthread_t hiloCpuPadre;
	int rtaHiloPadre;
	rtaHiloPadre = pthread_create(&hiloCpuPadre,NULL,(void*)conectarCPUPadreAPlanificador,NULL);
	if(rtaHiloPadre)
	{
		fprintf(stderr,"Error - pthread_create() return code: %d\n",rtaHiloPadre);
		printf("Se cerrara el programa");
		return EXIT_FAILURE;
	}
	pthread_join(hiloCpuPadre, NULL);
	return EXIT_SUCCESS;
}

int procesarInstruccion(char **lista, int32_t pid){
	if (string_equals_ignore_case(lista[0], "iniciar")){
		log_info(CPULog," [PID:%s] Instruccion: iniciar",string_itoa(pid));
		//lista[1] contiene la cantidad de paginas a pedir al AdminMemoria
		if(informarAdminMemoriaComandoIniciar(lista[1],pid)==EXIT_FAILURE) return EXIT_FAILURE;
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "finalizar")){
		log_info(CPULog," [PID:%s] Instruccion: finalizar",string_itoa(pid));
		//Informar al AdminMemoria que finalice el proceso
		informarAdminMemoriaComandoFinalizar(pid);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "leer")){
		log_info(CPULog," [PID:%s] Instruccion: leer",string_itoa(pid));
		//lista[1] contiene el nro de pagina
		informarAdminMemoriaComandoLeer(pid,lista[1]);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "escribir")){
		char* textoEscribir = string_from_format("%s",lista[2]);
		int32_t numeroPagina=*lista[1];
		log_info(CPULog," [PID:%s] Instruccion: escribir en página %s del proceso %s : %s",string_itoa(pid),numeroPagina, string_itoa(pid), textoEscribir);
		//lista[1] Contiene numero de página, lista[2] contiene el texto que se quiere a escribir en esa pagina.
		informarAdminMemoriaComandoEscribir(pid,numeroPagina,textoEscribir);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "entrada-salida")){
		int32_t tiempoDeEjec=*lista[1];
		log_info(CPULog," [PID:%s] Instruccion: entrada-salida en proceso %s de tiempo %s.", string_itoa(pid), string_itoa(pid), tiempoDeEjec);
		//lista[1] Contiene el tiempo que se debe bloquear.
		//informarAdminMemoriaComandoEntradaSalida(pid,tiempoDeEjec);
		sleep(configuracion->retardo);
		//informarPlanificadorLiberacionCPU(pid); //HACER FUNCION PARA INFORMAR AL PLANIFICADOR. PAGINA 5
	}else{
		log_warning(CPULog," [PID:%s] Instruccion: comando no interpretado",string_itoa(pid));
	}
	return EXIT_SUCCESS;
}
