#ifndef PAGINACION_H_
#define PAGINACION_H_
#include "frames.h"
#include "../virtual-memory/virtual-memory.h"
#include "../muse-structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>
#include <semaphore.h>

page* crear_pagina();
t_list* crear_tabla_de_paginas(int);
void agregar_paginas(t_list*,int,int);
int paginas_necesarias(int);
//int posicion_en_tabla_paginas(page*,t_list*);
void asignar_marco(page*);
void asignar_marco_en_swap(page*);
void swap_pages(page*,page*);
page* algoritmo_clock_modificado();
void escribir_en_archivo_swap(void*,t_list*,size_t,size_t);
void escribir_pagina_extra_en_archivo_swap(page*,t_list *);
void agregar_paginas_extras(t_list*, int);
void incrementar_indice();

#endif /* PAGINACION_H_ */
