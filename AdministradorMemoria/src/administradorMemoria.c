/*********** AD
}MINISTRADOR MEMORIA ************/

#include "administradorMemoria.h"

int main(void) {

	puts("Cargo archivo de configuracion de Administrador Memoria");
	t_Memoria_Config* config = cargarArchivoDeConfiguracion();

	if(config==NULL){
		printf("No se pudo cargar el archivo de configuración\n"); //se debería loguear, no printear
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
