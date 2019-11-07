/*
 * frames.h
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_FRAMES_H_
#define PAGINACION_FRAMES_H_
#define NO_FRAME -1
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "../paginacion/paginacion.h"
#include <stdbool.h>


t_list *FRAMES_TABLE;
t_bitarray *BIT_ARRAY_FRAMES;

typedef struct metadata_t{
	bool ocupado;
	int bytes;
}metadata;

typedef struct{
	int nro_frame;
	bool usado;
	t_list *metadatas;
	//void *memoria;
}frame;

void inicilizar_tabla_de_frames();
void dividir_memoria_en_frames(int, int);
void inicializar_bitmap();
frame* obtener_marco_libre();
void asignar_marcos(t_list*);
void asignar_marco(page*);
#endif /* PAGINACION_FRAMES_H_ */
