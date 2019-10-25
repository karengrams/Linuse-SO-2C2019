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

typedef struct{
	bool bit_presencia;
	int numero_frame;
}t_pagina;



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

#endif /* PAGINACION_H_ */
