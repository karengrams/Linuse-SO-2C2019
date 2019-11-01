/*
 * frames.h
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_FRAMES_H_
#define PAGINACION_FRAMES_H_

t_list *FRAMES_TABLE = list_create();
t_bitarray *BIT_ARRAY_FRAMES;

typedef struct{
	bool ocupado;
	int bytes;
}metadata;

/**
	* @NAME: dividir_memoria
	* @DESC: Divide la memoria en marcos del tam. de las paginas y se le agrega un metadata que indica
	* 		 la cantidad de bytes libres del segmento
	* @PARAMS:
	* 		void* memoria - puntero a memoria
	* 		int tamanioPagina - tam. de pagina a particionar memoria
	* 		int tamanioMemoria - tam. total de memoria
 */
void dividir_memoria_en_frames(void* memoria, int tamanioPagina, int tamanioMemoria);

void crear_bitmap();
#endif /* PAGINACION_FRAMES_H_ */
