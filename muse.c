#include "libMuse.c"
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>

t_config* archivo_config;

void leer_config(void){
	archivo_config = config_create("muse.config");
}

int memoria_del_disco(){
	int mem = atoi(config_get_string_value(archivo_config, "MEMORY_SIZE"))*8;
	free(archivo_config);
    return mem;
}

int main(void){
    printf("Wenas perri.\n");
    printf("Empecemos leyendo el .config..\n");
    leer_config();
    printf("Hagamos el malloc del disco para segmentacion..\n");
    void *memoria = (uint32_t*) malloc(memoria_del_disco());
    printf("Ahora si, empezamos con la segmentacion y paginacion..\n");
    comenzar_segmentacion_paginada();
    printf("El tamanio de memoria es de: %d bits \n",memoria_del_disco());
    return 0;
}

void comenzar_segmentacion_paginada(){

}
