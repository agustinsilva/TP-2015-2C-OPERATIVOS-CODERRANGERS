/*
 *  atencionPedidosCPU.c
 *  Created on: 20/9/2015
 *  Author: ElianaLS
 */
#include "administradorMemoria.h"

char* recibirMensaje(sock_t* socket)
{
	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje;
	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	recv(socket->fd, &longitudMensaje, sizeof(int32_t), 0);
	char* mensaje = (char*) malloc(longitudMensaje+1);
	recv(socket->fd, mensaje, longitudMensaje, 0);
	mensaje[longitudMensaje]='\0';
	return mensaje;
}

int32_t enviarMensaje(sock_t* socket, char* mensaje)
{
	/*prepara la longitud del archivo a mandar, así el receptor sabe cuánto recibir*/
	int32_t longitud = string_length(mensaje);
	int32_t status = send(socket->fd, &longitud, sizeof(int32_t),0);
	/*chequea envío*/
	if(!status)
	{
		printf("No se envió la cantidad de bytes a enviar luego\n");
		return status;
	}
	status = send(socket->fd, mensaje, longitud,0);
	return status;
}

int32_t hiloEjecucionCPU(t_HiloCPU* paramsCPU)
{
	log_info(MemoriaLog,"Esperando pedidos de Cpu \n");

	int32_t codigoOperacion;
	codigoOperacion = recibirCodigoOperacion(paramsCPU->cpuSocket);
	if(codigoOperacion==-1)	{
		log_error(MemoriaLog, RED"No se recibió correctamente el código de operación\n"RESET);
		return EXIT_FAILURE;
	}
	while(codigoOperacion!=0) 	{

		switch(codigoOperacion)	{
		case codigo_iniciar:
			iniciar(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		case codigo_finalizar:
			finalizar(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		case codigo_leer:
			lectura(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		case codigo_escribir:
			escritura(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		}

		codigoOperacion = recibirCodigoOperacion(paramsCPU->cpuSocket);
		if(codigoOperacion==-1) {
			log_error(MemoriaLog, RED"No se recibió correctamente el código de operación\n"RESET);
			return EXIT_FAILURE;
		}
	}

	if(codigoOperacion==0){
		log_info(MemoriaLog, "Se desconectó un CPU\n");
		bool PorCerrado(sock_t* socket){
			return socket->fd==0;
		}
		int32_t veces = list_count_satisfying(CPUsConectados,(void*)PorCerrado);
		int32_t it;
		for(it=0; it<veces; it++){
			list_remove_and_destroy_by_condition(CPUsConectados, (void*)PorCerrado, (void*)clean_socket);
		}
	}
	return 0;
}


int32_t recibirCodigoOperacion(sock_t* cpu)
{
	int32_t codigo;
	int32_t recibido = recv(cpu->fd, &codigo, sizeof(int32_t), 0);
	if(recibido!=sizeof(int32_t))
	{
		return -1;
	}
	else
	{
		return codigo;
	}
}

void iniciar(sock_t* cpuSocket, sock_t* swapSocket) {
	int32_t idmProc;
	int32_t cantPaginas;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);
	int32_t recibidoPags = recv(cpuSocket->fd, &cantPaginas, sizeof(int32_t), 0);

	if(recibidoProc <= 0 || recibidoPags <= 0)
	{
		log_error(MemoriaLog, RED "No se recibió correctamente la información de la CPU\n" RESET);
		enviarEnteros(cpuSocket, pedido_error);
		return;
	}
	else
	{
		log_info(MemoriaLog, "Se recibió de la CPU: PID: %d, Cant Paginas: %d\n", idmProc, cantPaginas);
	}

	pthread_mutex_lock(&sem_swap);
	enviarEnteros(swapSocket, codigo_iniciar);
	enviarEnteros(swapSocket, idmProc);
	enviarEnteros(swapSocket, cantPaginas);
	int32_t confirmacionSwap;
	int32_t recibidoSwap = recv(swapSocket->fd, &confirmacionSwap, sizeof(int32_t),0);
	pthread_mutex_unlock(&sem_swap);

	if(recibidoSwap <= 0)
	{
		log_error(MemoriaLog,RED"No se recibió correctamente la confirmación del Swap\n"RESET);
		enviarEnteros(cpuSocket, pedido_error);
		return;
	}

	  if(confirmacionSwap==pedido_exitoso){
		pthread_mutex_lock(&sem_TP);
	 	crearTablaDePaginas(idmProc,cantPaginas);
	 	pthread_mutex_unlock(&sem_TP);

	 	pthread_mutex_lock(&sem_stats);
		t_Stats* nuevo = malloc(sizeof(t_Stats));
		nuevo->idProc = idmProc;
		nuevo->pagsTotales = 0;
		nuevo->pageFaults = 0;

		list_add(estadisticas, nuevo);
		pthread_mutex_unlock(&sem_stats);

	  }

	enviarEnteros(cpuSocket, confirmacionSwap);
	if(confirmacionSwap==pedido_exitoso)
	{
		log_info(MemoriaLog,"-" BOLD " *Proceso nuevo* " RESET_NON_BOLD "Creado con éxito. PID: %d, Cantidad de Páginas: %d.\n", idmProc, cantPaginas);
	}
	else
	{
		if(confirmacionSwap == pedido_error)
		{
			log_error(MemoriaLog,RED" - *Proceso nuevo* Fallo al crear. Razón: No hay memoria disponible.\n"RESET);
		}
		else
		{
			log_error(MemoriaLog,RED" - *Proceso nuevo* Fallo al crear. Razón: Error de comunicación. \n"RESET);
		}
	}
}

void finalizar(sock_t* cpuSocket, sock_t* swapSocket)
{
	int32_t idmProc;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);

	if(recibidoProc <= 0){
		log_error(MemoriaLog,RED "No se recibió correctamente la información de la CPU\n" RESET);
		enviarEnteros(cpuSocket, pedido_error);
		return;
	}

	pthread_mutex_lock(&sem_swap);
	enviarEnteros(swapSocket, codigo_finalizar);
	enviarEnteros(swapSocket, idmProc);
	int32_t confirmacionSwap;
	int32_t recibidoSwap = recv(swapSocket->fd, &confirmacionSwap, sizeof(int32_t),0);
	pthread_mutex_unlock(&sem_swap);

	if(recibidoSwap <= 0)	{
		log_error(MemoriaLog,RED "No se recibió correctamente la confirmación del Swap\n" RESET);
		enviarEnteros(cpuSocket, pedido_error);
		return;
	}

	if(string_equals_ignore_case(configuracion->algoritmo_reemplazo, CLOCKM)){
		pthread_mutex_lock(&sem_order);
		eliminarOrdenMarcos(idmProc);
		pthread_mutex_unlock(&sem_order);
	}

	pthread_mutex_lock(&sem_TP);
	pthread_mutex_lock(&sem_MP);
	vaciarMarcosOcupados(idmProc);
	eliminarTablaDePaginas(idmProc);
	pthread_mutex_unlock(&sem_MP);
	pthread_mutex_unlock(&sem_TP);

	if(configuracion->tlb_habilitada){
		pthread_mutex_lock(&sem_TLB);
		eliminarDeTLBPorPID(idmProc);
		pthread_mutex_unlock(&sem_TLB);
	}

	pthread_mutex_lock(&sem_stats);
	t_Stats* estadistica = buscarEstadisticaPorProceso(idmProc);
	log_info(MemoriaLog," - " BOLD "*Fin de proceso*\n" RESET_NON_BOLD " PID: %d , Total Páginas Accedidas: %d , Fallos de Página: %d", idmProc, estadistica->pagsTotales, estadistica->pageFaults);
	pthread_mutex_unlock(&sem_stats);

	enviarEnteros(cpuSocket, confirmacionSwap);

	void printt(t_TP* entrada){
		if(entrada->idProc == idmProc){
			printf("No borre la pag %d del proceso %d\n", entrada->nroPag, entrada->idProc);
		}
	}
	list_iterate(tablasDePaginas, (void*)printt);
}

void lectura(sock_t* cpuSocket, sock_t* swapSocket){
	int32_t idmProc;
	int32_t nroPagina;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);
	int32_t recibidoPag = recv(cpuSocket->fd, &nroPagina, sizeof(int32_t), 0);

	if(recibidoProc <= 0 || recibidoPag <= 0) {
		log_error(MemoriaLog,RED"No se recibió correctamente la información de la CPU\n"RESET);
		return;
	}

	log_info(MemoriaLog, " - "BOLD "*Solicitud de Lectura*" RESET_NON_BOLD " PID: %d, Nro de Página: %d", idmProc, nroPagina);

	pthread_mutex_lock(&sem_stats);
	t_Stats* estadisticaProc = buscarEstadisticaPorProceso(idmProc);
	(estadisticaProc->pagsTotales)++;
	pthread_mutex_unlock(&sem_stats);

												/* SI HAY TLB */
	if(configuracion->tlb_habilitada==1){

		pthread_mutex_lock(&sem_TLB);
		t_TLB* entradaTLB = buscarEnTLB(idmProc, nroPagina);
		pthread_mutex_unlock(&sem_TLB);

		if(entradaTLB != NULL){    				/* TLB HIT */
			log_info(MemoriaLog, "-" GREEN " *TLB HIT*" RESET" Nro. Página: %d, Nro. Marco: %d \n", entradaTLB->pagina, entradaTLB->marco);

			pthread_mutex_lock(&sem_stats);
			t_Stats* estadisticaTLB = buscarEstadisticaPorProceso(stat_TLB);
			(estadisticaTLB->hit)++;
			pthread_mutex_unlock(&sem_stats);

			retardo(configuracion->retardo_memoria, memoria_principal, idmProc, entradaTLB->pagina, entradaTLB->marco);

			pthread_mutex_lock(&sem_MP);
			t_MP* hit = buscarEnMemoriaPrincipal(entradaTLB->marco);
			manejarMemoriaPrincipalLectura(hit, cpuSocket);
			pthread_mutex_unlock(&sem_MP);

		} else {      							/* TLB MISS */

			pthread_mutex_lock(&sem_stats);
			t_Stats* estadisticaTLB = buscarEstadisticaPorProceso(stat_TLB);
			(estadisticaTLB->miss)++;
			pthread_mutex_unlock(&sem_stats);

			retardo(configuracion->retardo_memoria, tabla_paginas, dummy, dummy, dummy);
			pthread_mutex_lock(&sem_TP);
			int32_t marco = buscarMarcoEnTablaDePaginas(idmProc, nroPagina);
			pthread_mutex_unlock(&sem_TP);

			switch(marco){
				case -1: {
					log_error(MemoriaLog, RED "Se intentó acceder a una página que no corresponde al proceso, o a un proceso ya finalizado. Para lectura PID: %d, Pag: %d\n" RESET, idmProc, nroPagina);
					enviarEnteros(cpuSocket, pedido_error);
					break;
				}
				case swap_in:{
					pthread_mutex_lock(&sem_TP);
					pthread_mutex_lock(&sem_MP);
					pthread_mutex_lock(&sem_swap);
					pthread_mutex_lock(&sem_TLB);
					pthread_mutex_lock(&sem_order);
					marco = swapIN(swapSocket, cpuSocket, idmProc, nroPagina, codigo_leer);
					pthread_mutex_unlock(&sem_order);
					pthread_mutex_unlock(&sem_TLB);
					pthread_mutex_unlock(&sem_swap);
					pthread_mutex_unlock(&sem_MP);
					pthread_mutex_unlock(&sem_TP);

					if(marco!=marcos_no_libres && marco!=marcos_insuficientes){
						pthread_mutex_lock(&sem_TLB);
						eliminarDeTLBPorMarco(marco);
						entradaTLB = actualizarTLB(idmProc,nroPagina,marco);

						if(marco!=marcos_no_libres && marco!=marcos_insuficientes && marco!=-1){
							if(entradaTLB!=NULL){
								log_info(MemoriaLog,"-" RED " *TLB MISS*" RESET" Nro. Página: %d, Nro. Marco: %d \n", entradaTLB->pagina, marco);
							}
						}
						pthread_mutex_unlock(&sem_TLB);
					}

//					void impr(t_MP* entrada){
//							printf("marco %d - ocupado: %d - contenido %s\n", entrada->marco, entrada->ocupado, entrada->contenido);
//						}
//					list_iterate(memoriaPrincipal, (void*)impr);

					pthread_mutex_lock(&sem_stats);
					(estadisticaProc->pageFaults)++;
					pthread_mutex_unlock(&sem_stats);
					break;
				}
				default:  /*buscar contenido en memoria principal*/ {
					retardo(configuracion->retardo_memoria, memoria_principal, idmProc, nroPagina, marco);

					pthread_mutex_lock(&sem_MP);
					pthread_mutex_lock(&sem_TLB);
					t_MP* miss = buscarEnMemoriaPrincipal(marco);
					manejarMemoriaPrincipalLectura(miss, cpuSocket);

					entradaTLB = actualizarTLB(idmProc, nroPagina, marco);

					if(marco!=marcos_no_libres && marco!=marcos_insuficientes && marco!=-1){
						if(entradaTLB!=NULL){
							log_info(MemoriaLog,"-" RED " *TLB MISS*" RESET" Nro. Página: %d, Nro. Marco: %d \n", entradaTLB->pagina, marco);
						}
					}

					pthread_mutex_unlock(&sem_TLB);
					pthread_mutex_unlock(&sem_MP);
					break;
				}
			}
		}
	} else {         							/* SI NO HAY TLB */

		retardo(configuracion->retardo_memoria, tabla_paginas, dummy, dummy, dummy);
		pthread_mutex_lock(&sem_TP);
		int32_t marco = buscarMarcoEnTablaDePaginas(idmProc, nroPagina);
		pthread_mutex_unlock(&sem_TP);

		switch(marco){
			case -1: log_error(MemoriaLog, RED "Se intentó acceder a una página que no corresponde al proceso, o a un proceso ya finalizado. Para lectura PID: %d, Pag: %d\n" RESET, idmProc, nroPagina);
			enviarEnteros(cpuSocket, pedido_error);
			break;
			case swap_in:{
				pthread_mutex_lock(&sem_TP);
				pthread_mutex_lock(&sem_MP);
				pthread_mutex_lock(&sem_swap);
				pthread_mutex_lock(&sem_order);
				swapIN(swapSocket, cpuSocket, idmProc, nroPagina, codigo_leer);
				pthread_mutex_unlock(&sem_order);
				pthread_mutex_unlock(&sem_swap);
				pthread_mutex_unlock(&sem_MP);
				pthread_mutex_unlock(&sem_TP);

				pthread_mutex_lock(&sem_stats);
				(estadisticaProc->pageFaults)++;
				pthread_mutex_unlock(&sem_stats);
				 break;
			}
			default:  /*buscar contenido en memoria principal*/ {
				retardo(configuracion->retardo_memoria, memoria_principal, idmProc, nroPagina, marco);
				pthread_mutex_lock(&sem_MP);
				t_MP* miss = buscarEnMemoriaPrincipal(marco);
				manejarMemoriaPrincipalLectura(miss, cpuSocket);
				pthread_mutex_unlock(&sem_MP);
				break;
			}
		}
	}

	pthread_mutex_lock(&sem_TP);
	avanzarTiempo(idmProc, nroPagina);
	pthread_mutex_unlock(&sem_TP);

	log_info(MemoriaLog,"Fin operación leer %d\n", nroPagina);
}

void escritura(sock_t* cpuSocket, sock_t* swapSocket){

	int32_t idmProc;
	int32_t nroPagina;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);
	int32_t recibidoPag = recv(cpuSocket->fd, &nroPagina, sizeof(int32_t), 0);

	if(recibidoProc <= 0 || recibidoPag <= 0) {
		log_error(MemoriaLog,RED"No se recibió correctamente la información de la CPU\n"RESET);
		return;
	}

	int32_t longitudContenido;
	int32_t recibidolongitudContenido = recv(cpuSocket->fd, &longitudContenido, sizeof(int32_t), 0);

	char* contenido = malloc(longitudContenido+1);
	int32_t recibidoContenido = recv(cpuSocket->fd, contenido, longitudContenido, 0);
	contenido[longitudContenido]='\0';

	if(recibidolongitudContenido <=0 || recibidoContenido<=0){
		log_error(MemoriaLog, RED"No se recibió correctamente el contenido de la página de CPU\n"RESET);
		return;
	}

	log_info(MemoriaLog, " - "BOLD "*Solicitud de Escritura*" RESET_NON_BOLD " PID: %d, Nro de Página: %d", idmProc, nroPagina);

	pthread_mutex_lock(&sem_stats);
	t_Stats* estadisticaProc = buscarEstadisticaPorProceso(idmProc);
	(estadisticaProc->pagsTotales)++;
	pthread_mutex_unlock(&sem_stats);

	/* SI HAY TLB */
	if(configuracion->tlb_habilitada==1){

		pthread_mutex_lock(&sem_TLB);
		t_TLB* entradaTLB = buscarEnTLB(idmProc, nroPagina);
		pthread_mutex_unlock(&sem_TLB);

		if(entradaTLB != NULL){    				/* TLB HIT */
			log_info(MemoriaLog, "-" GREEN " *TLB HIT*" RESET" Nro. Página: %d, Nro. Marco: %d \n", entradaTLB->pagina, entradaTLB->marco);

			pthread_mutex_lock(&sem_stats);
			t_Stats* estadisticaTLB = buscarEstadisticaPorProceso(stat_TLB);
			(estadisticaTLB->hit)++;
			pthread_mutex_unlock(&sem_stats);

			retardo(configuracion->retardo_memoria, memoria_principal, idmProc, entradaTLB->pagina, entradaTLB->marco);

			pthread_mutex_lock(&sem_MP);
			t_MP* hit = buscarEnMemoriaPrincipal(entradaTLB->marco);
			manejarMemoriaPrincipalEscritura(hit, cpuSocket, contenido, idmProc, nroPagina);
			pthread_mutex_unlock(&sem_MP);

		} else {      							/* TLB MISS */

			pthread_mutex_lock(&sem_stats);
			t_Stats* estadisticaTLB = buscarEstadisticaPorProceso(stat_TLB);
			(estadisticaTLB->miss)++;
			pthread_mutex_unlock(&sem_stats);

			retardo(configuracion->retardo_memoria, tabla_paginas, dummy, dummy, dummy);

			pthread_mutex_lock(&sem_TP);
			int32_t marco = buscarMarcoEnTablaDePaginas(idmProc, nroPagina);
			pthread_mutex_unlock(&sem_TP);

			switch(marco){
				case -1: {
					log_error(MemoriaLog, RED "Se intentó acceder a una página que no corresponde al proceso, o a un proceso ya finalizado. Para escritura PID: %d, Pag: %d\n" RESET, idmProc, nroPagina);
					enviarEnteros(cpuSocket, pedido_error);
					break;
				}
				case swap_in:{

					pthread_mutex_lock(&sem_TP);
					pthread_mutex_lock(&sem_MP);
					pthread_mutex_lock(&sem_swap);
					pthread_mutex_lock(&sem_TLB);
					pthread_mutex_lock(&sem_order);
					marco = swapIN(swapSocket, cpuSocket, idmProc, nroPagina, codigo_escribir);
					pthread_mutex_unlock(&sem_order);
					pthread_mutex_unlock(&sem_TLB);
					pthread_mutex_unlock(&sem_swap);
					pthread_mutex_unlock(&sem_MP);
					pthread_mutex_unlock(&sem_TP);

					if(marco!=marcos_no_libres && marco!=marcos_insuficientes){
						pthread_mutex_lock(&sem_TLB);
						eliminarDeTLBPorMarco(marco);
						entradaTLB = actualizarTLB(idmProc,nroPagina,marco);

						if(marco!=marcos_no_libres && marco!=marcos_insuficientes && marco!=-1){
							log_info(MemoriaLog,"-" RED " *TLB MISS*" RESET" Nro. Página: %d, Nro. Marco: %d \n", entradaTLB->pagina, marco);
						}
						pthread_mutex_unlock(&sem_TLB);

						retardo(configuracion->retardo_memoria, memoria_principal, idmProc, entradaTLB->pagina, entradaTLB->marco);

						pthread_mutex_lock(&sem_MP);
					    t_MP* miss = buscarEnMemoriaPrincipal(marco);
					    manejarMemoriaPrincipalEscritura(miss, cpuSocket, contenido, idmProc, nroPagina);
					    pthread_mutex_unlock(&sem_MP);
					}

//					void impr(t_MP* entrada){
//						printf("marco %d - ocupado: %d - contenido %s\n", entrada->marco, entrada->ocupado, entrada->contenido);
//					}
//					list_iterate(memoriaPrincipal, (void*)impr);

					pthread_mutex_lock(&sem_stats);
					(estadisticaProc->pageFaults)++;
					pthread_mutex_unlock(&sem_stats);
					break;
				}
				default:  /*buscar contenido en memoria principal*/ {

					retardo(configuracion->retardo_memoria, memoria_principal, idmProc, nroPagina, marco);

					pthread_mutex_lock(&sem_MP);
					pthread_mutex_lock(&sem_TLB);
					t_MP* miss = buscarEnMemoriaPrincipal(marco);
					manejarMemoriaPrincipalEscritura(miss, cpuSocket, contenido, idmProc, nroPagina);

					entradaTLB = actualizarTLB(idmProc, nroPagina, marco);

					if(marco!=marcos_no_libres && marco!=marcos_insuficientes && marco!=-1){
						log_info(MemoriaLog,"-" RED " *TLB MISS*" RESET" Nro. Página: %d, Nro. Marco: %d \n", entradaTLB->pagina, marco);
					}
					pthread_mutex_unlock(&sem_TLB);
					pthread_mutex_unlock(&sem_MP);
					break;
				}
			}
		}

	}
	else {         							/* SI NO HAY TLB */

		retardo(configuracion->retardo_memoria, tabla_paginas, dummy, dummy, dummy);

		pthread_mutex_lock(&sem_TP);
		int32_t marco = buscarMarcoEnTablaDePaginas(idmProc, nroPagina);
		pthread_mutex_unlock(&sem_TP);

		switch(marco){
			case -1: {
				log_error(MemoriaLog, RED "Se intentó acceder a una página que no corresponde al proceso, o a un proceso ya finalizado. Para lectura PID: %d, Pag: %d\n" RESET, idmProc, nroPagina);
				enviarEnteros(cpuSocket, pedido_error); break;
			}
			case swap_in:{
				pthread_mutex_lock(&sem_TP);
				pthread_mutex_lock(&sem_MP);
				pthread_mutex_lock(&sem_swap);
				pthread_mutex_lock(&sem_order);
				//CHEQUEAR MARCO DEVUELTO
				marco = swapIN(swapSocket, cpuSocket, idmProc, nroPagina, codigo_escribir);
				pthread_mutex_unlock(&sem_order);
				pthread_mutex_unlock(&sem_swap);
				pthread_mutex_unlock(&sem_MP);
				pthread_mutex_unlock(&sem_TP);

				retardo(configuracion->retardo_memoria, memoria_principal, idmProc, nroPagina, marco);
				pthread_mutex_lock(&sem_MP);
				t_MP* entrada = buscarEnMemoriaPrincipal(marco);
				manejarMemoriaPrincipalEscritura(entrada, cpuSocket, contenido, idmProc, nroPagina);
				pthread_mutex_unlock(&sem_MP);

				pthread_mutex_lock(&sem_stats);
				(estadisticaProc->pageFaults)++;
				pthread_mutex_unlock(&sem_stats);
				break;
			}
			default:  /*buscar contenido en memoria principal*/ {

				retardo(configuracion->retardo_memoria, memoria_principal, idmProc, nroPagina, marco);

				pthread_mutex_lock(&sem_MP);
				t_MP* miss = buscarEnMemoriaPrincipal(marco);
				manejarMemoriaPrincipalEscritura(miss, cpuSocket, contenido, idmProc, nroPagina);
				pthread_mutex_unlock(&sem_MP);
				break;
			}
		}
	}
	pthread_mutex_lock(&sem_TP);
	avanzarTiempo(idmProc, nroPagina);
	pthread_mutex_unlock(&sem_TP);

	free(contenido);
	log_info(MemoriaLog,"Fin operación escribir %d\n", nroPagina);
}

void enviarEnteros(sock_t* socket, int32_t entero)
{
	int32_t enviado = send(socket->fd, &entero, sizeof(int32_t), 0);
	if(enviado!=sizeof(int32_t))
	{
		printf("No se envió correctamente la información entera\n");
		return;
	}
}

void enviarStrings(sock_t* socket, char* string, int32_t longitud)
{
	int32_t enviado = send(socket->fd, string, longitud, 0);
	if(enviado!=longitud)
	{
		printf("No se envió correctamente el string\n");
	}
}

t_LecturaSwap* pedirPagina(sock_t* swapSocket, int32_t idmProc, int32_t nroPagina)
{
	t_LecturaSwap* pedido = malloc(sizeof(t_LecturaSwap));
	enviarEnteros(swapSocket, codigo_leer);
	enviarEnteros(swapSocket, idmProc);
	enviarEnteros(swapSocket, nroPagina);
	int32_t recibidoEncontro = recv(swapSocket->fd, &(pedido->encontro), sizeof(int32_t), 0);
/*	if(recibidoEncontro!=sizeof(int32_t))
*   {
*		printf("No se recibió correctamente la confirmación del Swap\n");
*		return NULL;
*	}
*/
	if(recibidoEncontro <= 0){
		printf("No se recibió correctamente la confirmación del Swap\n");
		return NULL;
	}
	if(pedido->encontro==false)	{
		return pedido;
	} else {
		int32_t recibidoCantidad = recv(swapSocket->fd, &(pedido->longitud), sizeof(int32_t), 0);
/*		if(recibidoCantidad!=sizeof(int32_t))
 * 		{
			printf("No se recibió correctamente el contenido de la página de Swap\n");
			return NULL;
		}*/
		if(recibidoCantidad <= 0)
		{
			printf("No se recibió correctamente el contenido de la página de Swap\n");
			return NULL;
		}
		pedido->contenido = malloc(pedido->longitud);
		int32_t recibidoContenido = recv(swapSocket->fd, pedido->contenido, pedido->longitud, 0);
		if(recibidoContenido <= 0)
		{
			printf("No se recibió correctamente el contenido de la página de Swap\n");
			return NULL;
		}
/*		if(recibidoContenido!=pedido->longitud)
 * 		{
			printf("No se recibió correctamente el contenido de la página de Swap\n");
			return NULL;
		}*/
		//Si pedido longitud es 1 es el caracter null es decir  \0.
		if(pedido->longitud > 1) {
			pedido->contenido[pedido->longitud-1] = '\0';
		}
		return pedido;
	}
}

void enviarContenidoPagina(sock_t* socket, t_LecturaSwap* pedido)
{
	enviarEnteros(socket, pedido_exitoso);
	enviarEnteros(socket, pedido->longitud);
	enviarStrings(socket, pedido->contenido, pedido->longitud);
}



