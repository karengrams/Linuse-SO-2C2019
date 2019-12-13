#ifndef PAGINACION_FRAMES_H_
#define PAGINACION_FRAMES_H_
#define NO_FRAME -1
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "../segmentacion/segmentacion.h"
#include "../structures.h"
#include "paginacion.h"
#include <stdint.h>
#include <stdbool.h>

void* memoria;
t_list *FRAMES_TABLE;
t_bitarray *BIT_ARRAY_FRAMES;

void inicilizar_tabla_de_frames();
void dividir_memoria_en_frames(void*,int, int);
void inicializar_bitmap();
frame* obtener_marco_libre();
void asignar_marcos(t_list*);
void* serializar_heap_metadata(heapmetadata* , int );


#endif /* PAGINACION_FRAMES_H_ */
