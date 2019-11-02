#ifndef SEGMENTACION_H
#define SEGMENTACION_H
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum segment_type_t{
	HEAP,
	MMAP,
} segment_type;

typedef struct segment_t{
	segment_type tipo;
	uint32_t baseLogica;
	int tamanio; //  tam. pedido
	t_list* tablaDePaginas;
} segment;

int minimo(int,int);
segment* crear_segmento(segment_type,uint32_t,int);
int posicion_en_tabla_segmento(segment*,t_list*);
bool segmento_puede_agrandarse(segment*,t_list*,int);
uint32_t limite_segmento(segment*);
bool segmento_tiene_espacio_disponible(segment*, int,uint32_t*);
bool tiene_espacio(void*,int, uint32_t*);
void desmappear_segmento(segment*, void*);
void mappear_segmento(segment*, void*);
segment* buscar_segmento_dada_una_direccion(t_list*,uint32_t);
void escribir_segmento(segment*, uint32_t , int , void* );
bool segmento_puede_escribirse(void* , int , int );
void reservar_memoria(int , uint32_t , segment* );
bool es_segmento_de_tipo_HEAP(segment*);


#endif /* muse-segmentacion_h */
