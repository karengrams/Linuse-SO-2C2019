#include "utils.h"
#include <stdlib.h>


t_list* dividir_memoria(void* memoria, int tamanioPagina, int tamanioMemoria){

	t_list* listaDeMarcos = list_create();
	int desplazamiento = 0;
	void* aux;

	while(desplazamiento < tamanioMemoria){
		aux = memoria+desplazamiento;
		list_add(listaDeMarcos, aux);
		desplazamiento += tamanioPagina;
	}

	return listaDeMarcos;

}

t_bitarray* crear_bitmap(t_list* listaDeMarcos){
	int bytes;
	int cantidadDeMarcos = list_size(listaDeMarcos);
	div_t aux = div(cantidadDeMarcos, 8);

		if (aux.rem == 0){
			bytes = aux.quot;
		} else {
			bytes = aux.quot + 1;
		}
	char* punteroABits = (char*)malloc(bytes);

	return bitarray_create_with_mode(punteroABits, bytes, LSB_FIRST);
}

t_proceso* crear_proceso(int id, char* ip){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->ip = malloc(sizeof(char)*16);
	strcpy(proceso->ip, ip);
	proceso->tablaDeSegmentos = list_create();
	return proceso;
}

t_segmento* crear_segmento(int type, void* baseLogica, int tamanio){
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->baseLogica = baseLogica;
	segmento->tamanio = tamanio;
	segmento->tipo = type;
	segmento->tablaDePaginas = list_create();
	return segmento;
}

