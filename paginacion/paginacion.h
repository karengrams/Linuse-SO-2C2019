/*
 * paginacion.h
 *
 *  Created on: 12 oct. 2019
 *      Author: utnso
 */

#ifndef PAGINACION_H_
#define PAGINACION_H_
#include "frames.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>

int TAM_PAG;

typedef struct t_page{
	bool bit_presencia;
	bool bit_modificado;
	bool bit_uso;
	int nro_frame; //Es necesario para buscarlo en el archivo swap si no esta cargado en memoria
	struct frame *frame; //Creo que esto no tiene sentido tenerlo
} __attribute__((packed)) page;

page* crear_pagina();
t_list* crear_tabla_de_paginas(int);
void agregar_paginas(t_list*,int);
int paginas_necesarias(int);
void asignar_marco(page*);
//uint32_t obtener_offset_de_tabla_de_pags_para_tam(t_list *, int );
//int obtener_indice_de_pag_con_espacio_disponible(t_list *,int );

#endif /* PAGINACION_H_ */
