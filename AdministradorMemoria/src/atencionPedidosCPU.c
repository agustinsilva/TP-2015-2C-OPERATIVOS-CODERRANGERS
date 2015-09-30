/*
 *  atencionPedidosCPU.c
 *  Created on: 20/9/2015
 *  Author: ElianaLS
 */
#include "administradorMemoria.h"


char* recibirMensaje(sock_t* socket){

	/*recibe la cantidad de bytes que va a tener el mensaje*/
	int32_t longitudMensaje;

	/*recibe el mensaje sabiendo cuánto va a ocupar*/
	recv(socket->fd, &longitudMensaje, sizeof(int32_t), 0);
	char* mensaje = (char*) malloc(longitudMensaje+1);
	recv(socket->fd, mensaje, longitudMensaje, 0);
	mensaje[longitudMensaje]='\0';
	return mensaje;
}

int32_t enviarMensaje(sock_t* socket, char* mensaje){

	/*prepara la longitud del archivo a mandar, así el receptor sabe cuánto recibir*/
	int32_t longitud = string_length(mensaje);
	int32_t status = send(socket->fd, &longitud, sizeof(int32_t),0);

	/*chequea envío*/
	if(!status){
		printf("No se envió la cantidad de bytes a enviar luego\n");
		return status;
	}
	status = send(socket->fd, mensaje, longitud,0);
	return status;
}

int32_t hiloEjecucionCPU(t_HiloCPU* paramsCPU){
	printf("Esperando pedidos de Cpu \n");

//	char mensajeCpu[1024];
//	int32_t status;
//	status = recv(paramsCPU->cpuSocket->fd, (void*)mensajeCpu, 1024, 0);
//	printf("Mensaje de cpu : %s \n",mensajeCpu);
//	/* Deberia validar lo que recibio */
//	/* prepara mensaje para enviar */
//	/*char* mensaje = "Hola Swap, soy el Admin de Memoria, mucho gusto ";*/
//	status = enviarMensaje(paramsCPU->swapSocket,mensajeCpu);
//
//	/*chequea envío*/
//	if(!status)	{
//		printf("No se envió el mensaje al swap\n");
//	}
//	else {
//		printf("Se envió a Swap: %s\n", mensajeCpu);
//	}
//	/*recibe la respuesta*/
//	char* respuesta = recibirMensaje(paramsCPU->swapSocket);
//	printf("Recibe respuesta: %s\n", respuesta);
//	free(respuesta);

	int32_t codigoOperacion;

	codigoOperacion = recibirCodigoOperacion(paramsCPU->cpuSocket);
	if(codigoOperacion==-1){
		printf("No se recibió correctamente el código de operación\n");
		return EXIT_FAILURE;
	}
	while(codigoOperacion!=0){
		switch(codigoOperacion){
		case codigo_iniciar: iniciar(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		case codigo_finalizar: finalizar(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		case codigo_leer: lectura(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		case codigo_escribir: escritura(paramsCPU->cpuSocket, paramsCPU->swapSocket);
			break;
		}
		codigoOperacion = recibirCodigoOperacion(paramsCPU->cpuSocket);
		if(codigoOperacion==-1){
			printf("No se recibió correctamente el código de operación\n");
			return EXIT_FAILURE;
		}
	}
	return 0;
}

int32_t recibirCodigoOperacion(sock_t* cpu){
	int32_t codigo;
	int32_t recibido = recv(cpu->fd, &codigo, sizeof(int32_t), 0);
	if(recibido!=sizeof(int32_t)){
		return -1;
	} else{
		return codigo;
	}
}

void iniciar(sock_t* cpuSocket, sock_t* swapSocket){
	printf("Se recibió el pedido de Iniciar Proceso\n");
	int32_t idmProc;
	int32_t cantPaginas;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);
	int32_t recibidoPags = recv(cpuSocket->fd, &cantPaginas, sizeof(int32_t), 0);

	if(recibidoProc!=sizeof(int32_t) || recibidoPags!=sizeof(int32_t)){
		printf("No se recibió correctamente la información de la CPU\n");
		enviarEnteros(cpuSocket, pedido_error);
		return;
	} else {
		printf("Se recibió de la CPU: PID: %d, Cant Paginas: %d\n", idmProc, cantPaginas);
	}

	enviarEnteros(swapSocket, codigo_iniciar);
	enviarEnteros(swapSocket, idmProc);
	enviarEnteros(swapSocket, cantPaginas);

	int32_t confirmacionSwap;
	int32_t recibidoSwap = recv(swapSocket->fd, &confirmacionSwap, sizeof(int32_t),0);
	if(recibidoSwap!=sizeof(int32_t)){
			printf("No se recibió correctamente la confirmación del Swap\n");
			enviarEnteros(cpuSocket, pedido_error);
			return;
	}

	/* No para este checkpoint
	 *
	 * if(confirmacionSwap==pedido_exitoso){
	 *	crearTablaDePaginas(idmProc,cantPaginas);
	 *  }
	 */
	enviarEnteros(cpuSocket, confirmacionSwap);

	if(confirmacionSwap==pedido_exitoso){
		log_info(MemoriaLog," - *Proceso nuevo* Creado con éxito \n  PID: %d, Cantidad de Páginas: %d \n", idmProc, cantPaginas);
	} else {

		if(confirmacionSwap == pedido_error){
			log_error(MemoriaLog," - *Proceso nuevo* Fallo al crear \n Razón: No hay memoria disponible\n");
		} else {
			log_error(MemoriaLog," - *Proceso nuevo* Fallo al crear \n Razón: Error de comunicación \n");
		}
	}


}

void finalizar(sock_t* cpuSocket, sock_t* swapSocket){
	int32_t idmProc;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);
	if(recibidoProc!=sizeof(int32_t)){
		printf("No se recibió correctamente la información de la CPU\n");
//		enviarEnteros(cpuSocket, pedido_error);
		return;
	}

	enviarEnteros(swapSocket, codigo_finalizar);
	enviarEnteros(swapSocket, idmProc);

	int32_t confirmacionSwap;
	int32_t recibidoSwap = recv(swapSocket->fd, &confirmacionSwap, sizeof(int32_t),0);
	if(recibidoSwap!=sizeof(int32_t)){
			printf("No se recibió correctamente la confirmación del Swap\n");
//			enviarEnteros(cpuSocket, pedido_error);
			return;
	}
/*
*	if(confirmacionSwap==pedido_exitoso){
*		eliminarTablaDePaginas(idmProc);
*		//liberar espacio
*	}
*/

	enviarEnteros(cpuSocket, confirmacionSwap);
	printf("Fin operación finalizar \n");
}


void lectura(sock_t* cpuSocket, sock_t* swapSocket){
	int32_t idmProc;
	int32_t nroPagina;
	int32_t recibidoProc = recv(cpuSocket->fd, &idmProc, sizeof(int32_t), 0);
	int32_t recibidoPag = recv(cpuSocket->fd, &nroPagina, sizeof(int32_t), 0);
	if(recibidoProc!=sizeof(int32_t) || recibidoPag!=sizeof(int32_t)){
		printf("No se recibió correctamente la información de la CPU\n");
		return;
	}

	log_info(MemoriaLog, " - *Solicitud de Lectura*  PID: %d, Nro de Página: %d\n", idmProc, nroPagina);

	if(configuracion->tlb_habilitada==1){
		/*
		 * buscar en TLB:
		 * Si esta    -> devolver contenido (logear hit con pag y frame resultante)
		 * Si no esta (logear miss y resultado alg. reemplazo)-> busca tabla de paginas -> busqueda en memoria principal
		 *                                         											Si esta en MP    -> devolver contenido
		 *                                        											Si no esta en MP -> pedir a Swap:
		 */
	} else{
		/* busqueda en memoria principal
				 * Si esta en MP    -> devolver contenido
				 * Si no esta en MP -> pedir a Swap:
				 */

		t_LecturaSwap* pedido = pedirPagina(swapSocket,idmProc, nroPagina);
		if(pedido==NULL || pedido->encontro==false){
			enviarEnteros(cpuSocket, pedido_error);
			return;
		}
		log_info(MemoriaLog, " - *Acceso a SWAP*  PID: %d\n", idmProc);

		/* actualizar memoria principal con frame/pagina y copiar contenido */

		enviarContenidoPagina(cpuSocket, pedido);

		free(pedido->contenido);
		free(pedido);
	}

	printf("Fin operación leer %d\n", nroPagina);
}

void escritura(sock_t* cpuSocket, sock_t* swapSocket){

}

void enviarEnteros(sock_t* socket, int32_t entero){
	int32_t enviado = send(socket->fd, &entero, sizeof(int32_t), 0);
	if(enviado!=sizeof(int32_t)){
		printf("No se envió correctamente la información entera\n");
		return;
	}
}

void enviarStrings(sock_t* socket, char* string, int32_t longitud){
	int32_t enviado = send(socket->fd, string, longitud, 0);
	if(enviado!=longitud){
		printf("No se envió correctamente el string\n");
		free(string);
		return;
	}
}

t_LecturaSwap* pedirPagina(sock_t* swapSocket, int32_t idmProc, int32_t nroPagina){

	t_LecturaSwap* pedido = malloc(sizeof(t_LecturaSwap));

	enviarEnteros(swapSocket, codigo_leer);
	enviarEnteros(swapSocket, idmProc);
	enviarEnteros(swapSocket, nroPagina);

	int32_t recibidoEncontro = recv(swapSocket->fd, &(pedido->encontro), sizeof(int32_t), 0);
/*	if(recibidoEncontro!=sizeof(int32_t)){
*		printf("No se recibió correctamente la confirmación del Swap\n");
*		return NULL;
*	}
*/
	if(pedido->encontro==false){
		return pedido;

	} else{

		int32_t recibidoCantidad = recv(swapSocket->fd, &(pedido->longitud), sizeof(int32_t), 0);
		if(recibidoCantidad!=sizeof(int32_t)){
			printf("No se recibió correctamente el contenido de la página de Swap\n");
			return NULL;
		}

		pedido->contenido = malloc(pedido->longitud);
		int32_t recibidoContenido = recv(swapSocket->fd, pedido->contenido, pedido->longitud, 0);
		if(recibidoContenido!=pedido->longitud){
			printf("No se recibió correctamente el contenido de la página de Swap\n");
			return NULL;
		}

		pedido->contenido[pedido->longitud-1] = '\0';
		return pedido;
	}
}

void enviarContenidoPagina(sock_t* socket, t_LecturaSwap* pedido){
	enviarEnteros(socket, pedido_exitoso);
	enviarEnteros(socket, pedido->longitud);
	enviarStrings(socket, pedido->contenido, pedido->longitud);
}
