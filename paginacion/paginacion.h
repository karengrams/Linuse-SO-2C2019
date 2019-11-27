#ifndef PAGINACION_H_
#define PAGINACION_H_
#include "frames.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>

typedef struct page_t page;

int TAM_PAG;

struct page_t{
	bool bit_presencia;
	bool bit_modificado;
	bool bit_uso;
	int nro_frame; //Es necesario para buscarlo en el archivo swap si no esta cargado en memoria
	struct frame *frame; //Creo que esto no tiene sentido tenerlo
} __attribute__((packed));

page* crear_pagina();
t_list* crear_tabla_de_paginas(int);
void agregar_paginas(t_list*,int);
int paginas_necesarias(int);
void cambiar_bit_de_prescencia(t_list *);
int posicion_en_tabla_paginas(page*,t_list*);
void asignar_marco(page*);
#endif /* PAGINACION_H_ */
