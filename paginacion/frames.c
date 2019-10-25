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
}


