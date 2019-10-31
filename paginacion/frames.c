/*
 * frames.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include <stdbool.h>
#include "frames.h"



void dividir_memoria_en_frames(void* memoria, int tamPag, int tamanioMemoria){
	t_list* listaDeMarcos = list_create();
	int desplazamiento = 0;
	void* aux;

	metadata* metadataInicial = malloc(sizeof(metadata));
	metadataInicial->bytes = tamPag;
	metadataInicial->ocupado = false;


	while(desplazamiento < tamanioMemoria){
		aux = memoria+desplazamiento;
		memcpy(memoria+desplazamiento,metadataInicial,sizeof(metadata));
		list_add(FRAMES_TABLE, aux);
		desplazamiento += tamPag;
	}
}



void crear_bitmap(){
		int bytes;
		int cantidadDeMarcos = list_size(FRAMES_TABLE);
		div_t aux = div(cantidadDeMarcos, 8);

			if (aux.rem == 0){
				bytes = aux.quot;
			} else {
				bytes = aux.quot + 1;
			}
		char *punteroABits = malloc(bytes);

		BIT_ARRAY_FRAMES = bitarray_create_with_mode(punteroABits, bytes, LSB_FIRST);
}



