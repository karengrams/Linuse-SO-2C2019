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

typedef struct{
	struct metadata ocupado;
	struct metadata libre;
	int nro_de_pag;
	bool bit_de_presencia;
}pagina;

typedef struct{
	uint32_t size;
	bool isFree;
}metadata;


void asignar_pagina(segmento seg);

#endif /* PAGINACION_H_ */
