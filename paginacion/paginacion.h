/*
 * paginacion.h
 *
 *  Created on: 12 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "segmentacion.h"
#include <stdio.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>

int NO_FRAME = -1;


typedef struct{
	bool bit_presencia;
	int numero_frame = NO_FRAME;
}page;

t_list* crear_lista_paginas(int cantidadDePaginas);

/**
	* @NAME: crear_bitmap
	* @DESC: crea el bitmap para definit el bit de prescencia
	* @PARAMS:
	* 		t_list* listaDeMarcos - puntero a tabla de marcos
**/
t_bitarray* crear_bitmap(t_list* listaDeMarcos);

/**
	* @NAME: paginas_necesarias
	* @DESC: segun un pedido define cuantas paginas seran necesarias designar
	* @PARAMS:
	* 		int valorPedido
**/
int paginas_necesarias(int valorPedido);

/**
	* @NAME: dividir_tabla_de_pags
	* @DESC: divide la tabla de paginas en una cantidad N que respeta el tamanio establecido.
	* 		 Para el valor pedido tener en cuenta el tamanio de las metadatas y sumarselo cuando se llama a esta funcion
	*		 (dependiendo el caso a veces no se agregan metadatas, otras veces una y otras veces dos metadatas...)
	* @PARAMS:
	* 		int cantidadDePaginas - cantidad de paginas a dividir la tabla
**/
void dividir_tabla_de_pags(int cantidadDePaginas, t_list *tabla_de_pags);

/**
	* @NAME: crear_pagina
	* @DESC: crea una pagina inicializando el bit array y su nro de frame.
	* @PARAMS:
	* 		N/A
**/
page* crear_pagina();

#endif /* PAGINACION_H_ */
