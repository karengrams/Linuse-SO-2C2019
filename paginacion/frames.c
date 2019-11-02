/*
 * frames.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include <stdbool.h>
#include "frames.h"

void dividir_memoria_en_frames(void* memoria, int pagetam, int memtam){
	int offset = 0;
	metadata *metadata_ptr;
	FRAMES_TABLE=list_create();
	while(offset<memtam){
		metadata_ptr = malloc(sizeof(metadata));
		(*metadata_ptr).bytes=pagetam;
		(*metadata_ptr).ocupado=false;
		list_add(FRAMES_TABLE,metadata_ptr);
		offset+=pagetam;
	}
	free(metadata_ptr);
}

void crear_bitmap(){
	int bytes;
	int cantidadDeMarcos = list_size(FRAMES_TABLE);
	div_t aux = div(cantidadDeMarcos, 8);

	if (aux.rem == 0){
		bytes = aux.quot;
	}else {
		bytes = aux.quot + 1;
	}
	char *punteroABits = malloc(bytes);
	BIT_ARRAY_FRAMES = bitarray_create_with_mode(punteroABits, bytes, LSB_FIRST);
	free(punteroABits);
}

int numero_marco_libre(){
	int marco=NO_FRAME; //Si no hay ningun marco libre devuelve -1
	for(int i = (bitarray_get_max_bit(BIT_ARRAY_FRAMES)-1); i>=0; i--){
		if(!bitarray_test_bit(BIT_ARRAY_FRAMES, i)) //Si esta vacio lo asignamos a j y lo devolvemos
			marco=i;
	}
	return marco;
}


