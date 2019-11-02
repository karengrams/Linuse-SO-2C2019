#ifndef SEGMENTACION_H
#define SEGMENTACION_H

#include <commons/collections/list.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


typedef enum segment_type_t{
	HEAP,
	MMAP,
} segment_type;

typedef struct segment_t{
	segment_type tipo;
	uint32_t baseLogica;
	int tamanio; //  tam. ocupado
	t_list* tablaDePaginas;
} segment;


segment* crear_segmento(segment_type,uint32_t,int);
int posicion_en_tabla_segmento(segment*,t_list*);
bool segmento_puede_agrandarse(segment*,t_list*,int);
uint32_t limite_segmento(segment*);

#endif /* muse-segmentacion_h */
