/*
 * split.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>

int main () {

	char comando[50];

	char** lista;


	printf ("Ingrese un comando\n");

	scanf("%s",comando);

	lista = string_split(comando," ");

	if (lista[0] == "correr")  /*o usar list_get?*/
		 printf("Se ejecutará el programa\n");
	else printf("Comando incorrecto\n");

return 0;
}
