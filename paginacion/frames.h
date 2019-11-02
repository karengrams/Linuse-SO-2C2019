/*
 * frames.h
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_FRAMES_H_
#define PAGINACION_FRAMES_H_
#define NO_FRAME -1
t_list *FRAMES_TABLE;
t_bitarray *BIT_ARRAY_FRAMES;

typedef struct{
	bool ocupado;
	int bytes;
}metadata;

void dividir_memoria_en_frames(void*, int, int);
/*
void dividir_memoria_en_frames(void* memoria, int tamanioPagina, int tamanioMemoria);
void crear_bitmap();
*/
#endif /* PAGINACION_FRAMES_H_ */
