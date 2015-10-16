/***********CPU************/

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
}

/*void crearHilosCPU()
{
	int rtaHilo = 0;
	int cantidad = 1;
	t_list* listaCPU = list_create();

	while (cantidad <= configuracion->cantidadHilos){

		pthread_t hiloCpu;
		rtaHilo = pthread_create(&hiloCpu,NULL,(void*)escucharYAtender,NULL);

		if(rtaHilo!=0)
				{
					fprintf(stderr,"Error - pthread_create() return code: %d\n",rtaHilo);
					printf("Se cerrara el programa");
					exit(EXIT_FAILURE);
				}
		else {
			list_add(listaCPU, (void*)hiloCpu);
			//AGREGA EN UNA LISTACPU TODOS LOS HILOS QUE SE CREARON EN BASE A EL ARCHIVO DE CONFIG.
		}

		cantidad++;
	}
}*/


int abrirArchivoYValidar(t_pcb* pcb){
	char* resultadosDeEjecuciones =  string_new();
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
	if(conectarAAdministradorMemoria()){
		return EXIT_FAILURE;
	}
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
				char* rta = procesarInstruccion(lista,pcb->idProceso,resultadosDeEjecuciones);
				if(string_equals_ignore_case(rta, "FIN")){
					break;//Termina la ejecucion porque:bloqueo de E/S
				}else{
					string_append(&resultadosDeEjecuciones,rta);
				}
				cantInstruccionesEjecutadas++;
				QUANTUMRESTANTE--;
			}/*else{
				break;//Termina la ejecucion porque: terminó de ejecutar todas las instrucciones
			}*/
		}
		if(QUANTUMRESTANTE == 0){
			log_info(CPULog," [PID:%s] Finalizó quantum de ejecución.\n",string_itoa(pcb->idProceso));
			//actualizamos el puntero del pcb
			pcb->contadorPuntero = pcb->contadorPuntero + cantInstruccionesEjecutadas + 1;
			//enviamos el pcb al planificador ya que terminó de ejecutar su quantum
			informarPlanificadorLiberacionCPU(pcb,resultadosDeEjecuciones);
		}
	}else{//FIFO
		printf("Es FIFO\n");
		while(fgets(instruccion,TAMINSTRUCCION+1, entrada) != NULL) {
			lista = string_split(instruccion," ");
			char* rta = procesarInstruccion(lista,pcb->idProceso,resultadosDeEjecuciones);
			if(string_equals_ignore_case(rta, "FIN")){
				break;//Termina la ejecucion porque: bloqueo de E/S; terminó el archivo
			}else{
				string_append(&resultadosDeEjecuciones,rta);
			}
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

char* procesarInstruccion(char **lista, int32_t pid, char* resultadosDeEjecuciones){
	char* rta;
	if (string_equals_ignore_case(lista[0], "iniciar")){
		log_info(CPULog," [PID:%s] Instruccion: iniciar",string_itoa(pid));
		//lista[1] contiene la cantidad de paginas a pedir al AdminMemoria
		rta = informarAdminMemoriaComandoIniciar(lista[1],pid);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "finalizar")){
		log_info(CPULog," [PID:%s] Instruccion: finalizar",string_itoa(pid));
		//Informar al AdminMemoria que finalice el proceso
		rta = informarAdminMemoriaComandoFinalizar(pid,resultadosDeEjecuciones);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "leer")){
		log_info(CPULog," [PID:%s] Instruccion: leer",string_itoa(pid));
		//lista[1] contiene el nro de pagina
		rta = informarAdminMemoriaComandoLeer(pid,lista[1]);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "escribir")){
		char* textoEscribir = string_from_format("%s",lista[2]);
		int32_t numeroPagina=*lista[1];
		log_info(CPULog," [PID:%s] Instruccion: escribir en página %s: %s",string_itoa(pid),numeroPagina,textoEscribir);
		//lista[1] Contiene numero de página, lista[2] contiene el texto que se quiere a escribir en esa pagina.
		rta = informarAdminMemoriaComandoEscribir(pid,numeroPagina,textoEscribir);
		sleep(configuracion->retardo);
	}else if(string_equals_ignore_case(lista[0], "entrada-salida")){
		int32_t tiempoDeEjec=*lista[1];
		log_info(CPULog,"[PID:%s] Instruccion: entrada-salida de tiempo %s.", string_itoa(pid), string_itoa(tiempoDeEjec));
		//lista[1] Contiene el tiempo que se debe bloquear.
		sleep(configuracion->retardo);
		rta = informarEntradaSalida(pid,tiempoDeEjec,resultadosDeEjecuciones);
	}else{
		log_warning(CPULog," [PID:%s] Instruccion: comando no interpretado",string_itoa(pid));
		rta = "Comando no interpretado.\n";
	}
	return rta;
}
