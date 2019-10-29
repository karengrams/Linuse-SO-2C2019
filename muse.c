#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../segmentacion/segmentacion.c"
#include "../paginacion/paginacion.c"
#include "../paginacion/frames.c"
#include "../utils/utils.c"
#include "../utils/utilsSockets.c"

t_list* PROCESS_TABLE = list_create();
t_config* archivo_config;

void leer_config(void){
	archivo_config = config_create("/home/utnso/Escritorio/muse.config");
}

int leer_del_config(char* valor){
	return config_get_int_value(archivo_config,valor);
}

int main(void){
	// printf("Wenas perri.\n");
	//printf("Empecemos leyendo el .config..\n");
    leer_config();
    //printf("Hagamos el malloc del disco para segmentacion..\n");
    //printf("El tamanio de memoria es de: %d bytes \n",leer_del_config("MEMORY_SIZE"));
    //printf("El tamanio de la pagina es de: %d bytes \n",leer_del_config("PAGE_SIZE"));
    void *memoria =  malloc(leer_del_config("MEMORY_SIZE"));
    int tam_mem = leer_del_config("MEMORY_SIZE");
    int tam_pag = leer_del_config("PAGE_SIZE");
    char* puerto = config_get_string_value("LISTEN_PORT");
    int cantidad_de_pags = tam_mem/tam_pag;
    dividir_memoria_en_frames(memoria, tam_pag, tam_mem);
    crear_bitmap();

    //Arranca a atender clientes

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
	}
    return 0;
}

