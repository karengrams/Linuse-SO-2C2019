/*
 * frames.h
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_FRAMES_H_
#define PAGINACION_FRAMES_H_


t_list* FRAMES_TABLE = list_create();


/**
 	*  Un frame siempre tiene un metadata que sera el espacio libre del mismo. Siempre se inicializara
 	* uno libre con el tam. de una pagina
**/
typedef struct{
	struct metadata metadata;
} frame;

typedef struct{
	bool ocupado;
	int bytes;
} metadata;

/**
	* @NAME: dividir_memoria
	* @DESC: Divide la memoria en marcos del tam. de las paginas y se le agrega un metadata que indica
	* 		 la cantidad de bytes libres del segmento
	* @PARAMS:
	* 		void* memoria - puntero a memoria
	* 		int tamanioPagina - tam. de pagina a particionar memoria
	* 		int tamanioMemoria - tam. total de memoria
 */
void dividir_memoria(void* memoria, int tamanioPagina, int tamanioMemoria);

#endif /* PAGINACION_FRAMES_H_ */
