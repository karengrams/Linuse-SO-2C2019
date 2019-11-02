/*
 * paginacion.h
 *
 *  Created on: 12 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "../segmentacion/segmentacion.h"
#include <stdio.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>

int TAM_PAG;

typedef struct page_t{
	bool bit_presencia;
	int numero_frame;
}page;

t_list* crear_lista_paginas(int);
void agregar_paginas(t_list*,int);
int paginas_necesarias(int);
page* crear_pagina();
void dividir_tabla_de_pags(int, t_list*);

#endif /* PAGINACION_H_ */
