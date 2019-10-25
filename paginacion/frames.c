/*
 * frames.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include <stdbool.h>
#include "frames.h"

void dividir_memoria(void* memoria, int tamPag, int tamanioMemoria){
	int desplazamiento = 0;
	metadata* aux;

	while(desplazamiento < tamanioMemoria){
		aux = (metadata*)malloc(sizeof(metadata));
		aux->bytes = tamPag;
		aux->ocupado=false;
		list_add(FRAMES_TABLE, aux);
		desplazamiento += tamPag;
	}

t_bitarray* crear_bitmap(){
		int bytes;
		int cantidadDeMarcos = list_size(FRAMES_TABLE);
		div_t aux = div(cantidadDeMarcos, 8);

			if (aux.rem == 0){
				bytes = aux.quot;
			} else {
				bytes = aux.quot + 1;
			}
		char* punteroABits = (char*)malloc(bytes);

		return bitarray_create_with_mode(punteroABits, bytes, LSB_FIRST);
	}
}


