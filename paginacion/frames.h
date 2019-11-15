#ifndef PAGINACION_FRAMES_H_
#define PAGINACION_FRAMES_H_
#define NO_FRAME -1
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "paginacion.h"
#include <stdint.h>
#include <stdbool.h>
#include "../segmentacion/segmentacion.h"
#include "paginacion.h"

t_list *FRAMES_TABLE;
t_bitarray *BIT_ARRAY_FRAMES;

typedef struct frame_t{
	void *memoria;
	int nro_frame;
}__attribute__((packed)) frame;

void inicilizar_tabla_de_frames();
void dividir_memoria_en_frames(void*,int, int);
void inicializar_bitmap();
frame* obtener_marco_libre();
void asignar_marcos(t_list*);
//int espacio_libre_de_frame(frame*);
//uint32_t obtener_offset_de_metadatas(t_list *);

#endif /* PAGINACION_FRAMES_H_ */
