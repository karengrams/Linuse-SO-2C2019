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

sem_t mutex_write_frame;
sem_t mutex_clock_mod;
sem_t mutex_frames;
sem_t binary_swap_pages;


t_list* PAGINAS_EN_FRAMES; //vector con paginas que estan presentes en frames para clock modificado
int TAM_PAG;

page* crear_pagina();
t_list* crear_tabla_de_paginas(int);
void agregar_paginas(t_list*,int,int);
int paginas_necesarias(int);
void asignar_marco(page*);
void asignar_marco_en_swap(page*);
void traer_pagina(page*);
void swap_pages(page*,page*);
page* algoritmo_clock_modificado();
void escribir_en_archivo_swap(void*,t_list*,size_t,size_t);
void escribir_pagina_extra_en_archivo_swap(page*,t_list *);
void agregar_paginas_extras(t_list*, int);
void incrementar_indice();
void eliminar_pagina_de_segmento(segment *, page *);
void eliminar_pagina(void*);
void agregar_a_lista_de_marcos_en_paginas(page*);
page * obtener_pagina_en_frames_segun(int);


#endif /* PAGINACION_H_ */
