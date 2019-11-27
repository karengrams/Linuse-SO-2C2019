#ifndef SEGMENTACION_H
#define SEGMENTACION_H
#include <commons/collections/list.h>
#include "../paginacion/frames.h"
#include "../paginacion/paginacion.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


typedef enum segment_type_t {
	HEAP, MMAP,
} segment_type;

typedef struct segment_t {
	segment_type tipo;
	uint32_t base_logica;
	int tamanio; //  tam. pedido
	t_list *metadatas; // Hay dos tipos segmentmetadata o mmapmetadata
	t_list *tabla_de_paginas;
}__attribute__((packed)) segment;

typedef struct heapmetadata_t{
	bool ocupado;
	int bytes;
}__attribute__((packed)) heapmetadata;

typedef struct segmentmetadata_t{
	heapmetadata *metadata;
	uint32_t posicion_inicial;
}__attribute__((packed)) segmentmetadata;

typedef struct mmapmetadata_t{
	char *path;
	uint32_t posicion_inicial;
	void *ptr_file;
	int bytes;
	int flags;
}__attribute__((packed)) mmapmetadata;

int minimo(int, int);
segment* crear_segmento(segment_type, int, t_list*);
uint32_t calculo_base_logica(segment *, t_list*);
segment* buscar_segmento_heap_para_tam(t_list*, int);
segment* buscar_segmento_heap_expandible_para_tam(t_list*, int);
bool segmento_de_tipo_heap_y_expandible(int, t_list*, void*);
bool segmento_de_tipo_heap_y_con_espacio(int, void*);
bool tiene_espacio_suficiente(int ,void*);
int posicion_en_tabla_segmento(segment*, t_list*);
uint32_t limite_segmento(segment*);
bool segmento_puede_agrandarse(segment*, int, t_list*);
int numero_pagina(segment*, uint32_t);
int espacio_libre(segment *);
segment* buscar_segmento_dada_una_direccion(uint32_t, t_list*);
uint32_t obtener_offset_para_tam(segment *, int );
void expandir_segmento(segment *,int);
bool metadatas_fusionables(segmentmetadata *, segmentmetadata *);
segmentmetadata* buscar_metadata_para_anidar(t_list *, segmentmetadata *);
int index_del_segment_metadata(segmentmetadata *, t_list *);
void buddy_system(segmentmetadata *, t_list *);
void mostrar_metadatas(t_list*);
void mostrar_metadatas_mmap(t_list*);
void mostrar_segmentos(t_list *);
void mostrar_tabla_de_segmentos(t_list*);

#endif /* muse-segmentacion_h */
