#include <commons/config.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/bitarray.h>
#include "segmentacion/segmentacion.h"
#include "paginacion/frames.h"
#include "paginacion/paginacion.h"
#include "utils/utilsSockets.h"
#include "utils/utils.h"
#include <stdlib.h>
#include <string.h>


t_config* leer_config(){
	return config_create("/home/utnso/Escritorio/muse.config");
}

int leer_del_config(char* valor,t_config* archivo_config){
	return config_get_int_value(archivo_config,valor);
}

void destruccion_tabla_de_marcos() {
	void _destroy_element(void *elemento) {

		void _destroy_metadata(void*elemento){
			free((metadata*)elemento);
		}

		frame *marco=(frame*)elemento;
		list_destroy_and_destroy_elements((*marco).metadatas,&_destroy_metadata);
		free(marco);
	}
	list_destroy_and_destroy_elements(FRAMES_TABLE, &_destroy_element);
	free(BIT_ARRAY_FRAMES->bitarray);
	bitarray_destroy(BIT_ARRAY_FRAMES);
}

int main(void){
	inicilizar_tabla_de_frames();
	inicializar_bitmap();
	t_config* config = leer_config();
	void *memoria = (void*)malloc(leer_del_config("MEMORY_SIZE",config));
	dividir_memoria_en_frames(leer_del_config("PAGE_SIZE",config),leer_del_config("MEMORY_SIZE",config));
	destruccion_tabla_de_marcos();
	free(memoria);
	config_destroy(config);

    //Arranca a atender clientes
    /*
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal para lectura de descriptores de fichero para select()
	int fdmax;        // Ultimo socket recibido
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	int socketEs = iniciar_socket_escucha("127.0.0.1", puerto); // obtener socket para listen

    FD_SET(socketEs, &master);// añadir socketEscucha al conjunto maestro
    fdmax = socketEs;

    while(1){
       read_fds = master;
       select(fdmax+1, &read_fds, NULL, NULL, NULL); // @suppress("Symbol is not resolved")
     for(int i = 0; i <= fdmax; i++) { // explorar conexiones existentes en busca de datos que leer
         if (FD_ISSET(i, &read_fds)) { //Hay datos que leer...
           if (i == socketEs) { //si se recibe en el socket escucha hay nuevas conexiones que aceptar
        	   	   	admitirNuevoCliente(&master, &fdmax, i); //agregar al master los nuevos clientes
           	   	  } else {
           	   		atenderCliente(&master, i); //leer mensajes y paquetes e imprimirlos por consola
           	   	  }
         	 }
     	 }
	}*/
    return 0;
}

