/*
 * paginacion.h
 *
 *  Created on: 12 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "../segmentacion/segmentacion.h"
#include "frames.h"

#include <stdio.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>

int TAM_PAG;

typedef struct{
	int nro_pag;
	bool bit_presencia;
	struct frame *frame;
}page;

page* crear_pagina();

t_list* crear_tabla_de_paginas(int);
void agregar_paginas(t_list*,int);
int paginas_necesarias(int);

#endif /* PAGINACION_H_ */
