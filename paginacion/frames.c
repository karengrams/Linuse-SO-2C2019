/*
 * frames.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include <stdbool.h>
#include "frames.h"

void dividir_memoria_en_frames(void* memoria, int tamPag, int tamanioMemoria){

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
	free(metadataInicial);
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

int numero_marco_libre(){
	int j = -1; //Si no hay ningun marco libre devuelve -1
	for(int i = (bitarray_get_max_bit(BIT_ARRAY_FRAMES)-1); i>=0; i--){
		if(!bitarray_test_bit(BIT_ARRAY_FRAMES, i)) //Si esta vacio lo asignamos a j y lo devolvemos
			j=i;
	}
	return j;
}

void asignar_marco_a_pagina(page* pagina, int index){
	bitarray_set_bit(BIT_ARRAY_FRAMES, index);
	pagina->numero_frame = index;
}

void asignar_marcos(t_list* paginas_a_asignar){
	int paginas = paginas_a_asignar->elements_count;
	for(int i=0;i<=paginas;i++){
		asignar_marco_a_pagina(paginas_a_asignar->head->data+i,i);
	}
}
