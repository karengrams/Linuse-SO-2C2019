#include "segmentacion.h"
#include "../paginacion/paginacion.h"
#include "../paginacion/frames.h"

int minimo(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

segment* crear_segmento(segment_type tipo, int tam, t_list* tabla_de_segmentos) {
	segment *segmento_ptr = (segment*) malloc(sizeof(segment));
	list_add(tabla_de_segmentos, segmento_ptr);
	(*segmento_ptr).tabla_de_paginas = crear_tabla_de_paginas(tam);
	asignar_marcos((*segmento_ptr).tabla_de_paginas);
	(*segmento_ptr).base_logica = calculo_base_logica(segmento_ptr,
			tabla_de_segmentos);
	(*segmento_ptr).tipo = tipo;
	(*segmento_ptr).metadatas = list_create();
	(*segmento_ptr).tamanio = 0;

	return segmento_ptr;
}

uint32_t calculo_base_logica(segment *segmento_ptr, t_list *tabla_de_segmentos) {
	int pos_seg = posicion_en_tabla_segmento(segmento_ptr, tabla_de_segmentos);
	segment* segmento_anterior = (segment*) list_get(tabla_de_segmentos,
			pos_seg - 1);
	if (!segmento_anterior) {
		return 0; // Si no hay ningun segmento atras
	}
	return (*segmento_anterior).base_logica
			+ (*segmento_anterior).tabla_de_paginas->elements_count * TAM_PAG;
}

segment* buscar_segmento_heap_para_tam(t_list* tabla_de_segmentos, int tam) {

	bool _segmento_de_tipo_heap_que_tenga_espacio_suficiente(void*element) {
		return segmento_de_tipo_heap_y_con_espacio(tam, element);
	}

	return (segment*) list_find(tabla_de_segmentos,
			_segmento_de_tipo_heap_que_tenga_espacio_suficiente);
}

segment* buscar_segmento_heap_expandible_para_tam(t_list* tabla_de_segmentos,
		int tam) {

	bool _segmento_de_tipo_heap_que_se_pueda_expandir(void*element) {
		return segmento_de_tipo_heap_y_expandible(tam, tabla_de_segmentos,
				element);
	}

	return (segment*) list_find(tabla_de_segmentos,
			_segmento_de_tipo_heap_que_se_pueda_expandir);

}

bool segmento_de_tipo_heap_y_expandible(int tam, t_list* tabla_de_segmentos,
		void*element) {
	segment *ptr_segmento = (segment*) element;
	return ptr_segmento->tipo == HEAP
			&& segmento_puede_agrandarse(ptr_segmento, tam, tabla_de_segmentos);
}

bool segmento_de_tipo_heap_y_con_espacio(int tam, void*element) {
	segment *ptr_segmento = (segment*) element;

	bool _tiene_espacio_suficiente(void*element) {
		return tiene_espacio_suficiente(tam, element);
	}

	bool tiene_espacio = list_any_satisfy(ptr_segmento->metadatas,
			_tiene_espacio_suficiente);

	if (list_is_empty(ptr_segmento->metadatas))
		tiene_espacio = true;

	return ptr_segmento->tipo == HEAP && tiene_espacio;
}

bool tiene_espacio_suficiente(int tam, void*element) {
	segmentmetadata *ptr_seg_metadata = (segmentmetadata*) element;
	return (ptr_seg_metadata->metadata->bytes) >= (tam + sizeof(heapmetadata))
			&& !ptr_seg_metadata->metadata->ocupado;
}

int posicion_en_tabla_segmento(segment* elemento, t_list *tabla_de_segmentos) {
	segment *comparador;
	for (int index = 0; index < tabla_de_segmentos->elements_count; index++) {
		comparador = list_get(tabla_de_segmentos, index);
		if (!memcmp(elemento, comparador, sizeof(segment))) { //Si son iguales devuelve 0
			return index;
		}
	}
	return -1;
}

uint32_t limite_segmento(segment* segmento) {
	return (segmento->base_logica
			+ list_size(segmento->tabla_de_paginas) * TAM_PAG) - 1;
}

// TODO:Habria que probar si funciona cuando haya un MMAP adelante
bool segmento_puede_agrandarse(segment* segmento, int valorPedido,
		t_list*tabla_de_segmentos) {
	int pos_seg = posicion_en_tabla_segmento(segmento, tabla_de_segmentos);
	segment *siguiente = (segment*) list_get(tabla_de_segmentos, (pos_seg + 1)); // TODO: si el segmento se libera, se toma un segmento inexistente?

	segmentmetadata *ptr_seg_metadata = (segmentmetadata*) list_get(
			segmento->metadatas, segmento->metadatas->elements_count - 1);
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;

	int paginasNecesarias = paginas_necesarias(
			valorPedido + sizeof(heapmetadata) - ptr_metadata->bytes);
	if (!siguiente) {
		return true; //Es el ultimo elemento de la lista
	}
	if (((segmento->base_logica
			+ (segmento->tabla_de_paginas->elements_count + paginasNecesarias)
					* TAM_PAG) - 1) < siguiente->base_logica)
		return true;

	return false;
}

int numero_pagina(segment* segmento, uint32_t direccion) {
	int desplazamientoEnSegmento = direccion - segmento->base_logica;
	return div(desplazamientoEnSegmento, TAM_PAG).quot;
}

int desplazamiento_en_pagina(segment* segmento, uint32_t direccion) {
	int desplazamientoEnSegmento = direccion - segmento->base_logica;
	return div(desplazamientoEnSegmento, TAM_PAG).rem;
}

//TODO: ver caso de error
int tamanio_segmento(segment* segmento) {
	return ((list_size(segmento->tabla_de_paginas)) * (TAM_PAG));
}

segment* buscar_segmento_dada_una_direccion(uint32_t direccion,
		t_list*tabla_de_segmentos) {
	bool _esta_dentro_del_limite(void *element) {
		segment *ptr_segmento = (segment*) element;
		return ptr_segmento->base_logica <= direccion
				&& limite_segmento(ptr_segmento) >= direccion;
	}

	return (segment*) list_find(tabla_de_segmentos, &_esta_dentro_del_limite);
}

uint32_t obtener_offset_para_tam(segment *segmento, int tam) {
	uint32_t direccion = 0;
	t_list *paux_metadatas = segmento->metadatas;
	if (list_is_empty(paux_metadatas)) {
		return direccion;
	} else {
		for (int i = 0; i < paux_metadatas->elements_count; i++) {
			segmentmetadata* ptr_seg_metadata = (segmentmetadata*) list_get(
					paux_metadatas, i);
			if ((!ptr_seg_metadata->metadata->ocupado)
					&& (ptr_seg_metadata->metadata->bytes
							>= (tam + sizeof(heapmetadata))
							|| ptr_seg_metadata->metadata->bytes == tam))
				direccion = ptr_seg_metadata->posicion_inicial;
		}
	}
	return direccion;
}

segmentmetadata *buscar_metadata_para_liberar(uint32_t direccion,
		segment *segmento) {
	bool _direccion_de_metadata(void* element) {
		segmentmetadata *ptr_seg_metadata = ((segmentmetadata*) element);
		uint32_t direccion_seg_metadata = sizeof(heapmetadata)
				+ ptr_seg_metadata->posicion_inicial;
		return direccion_seg_metadata == direccion;
	}
	return (segmentmetadata*) list_find(segmento->metadatas,
			_direccion_de_metadata);

}

segmentmetadata* buscar_metadata_de_segmento_segun(uint32_t offset,
		segment* segmento) {
	bool _direccion_de_metadata(void* element) {
		return ((segmentmetadata*) element)->posicion_inicial == offset;
	}
	segmentmetadata *ptr_seg_metadata;

	if (list_is_empty(segmento->metadatas)) {
		ptr_seg_metadata = (segmentmetadata*) malloc(sizeof(segmentmetadata));
		heapmetadata *ptr_metadata = (heapmetadata*) malloc(
				sizeof(heapmetadata));
		ptr_metadata->bytes = (segmento->tabla_de_paginas->elements_count
				* TAM_PAG) - sizeof(heapmetadata) - 1;
		ptr_metadata->ocupado = false;
		ptr_seg_metadata->metadata = ptr_metadata;
		ptr_seg_metadata->posicion_inicial = offset;
		list_add(segmento->metadatas, ptr_seg_metadata);
	} else {
		ptr_seg_metadata = (segmentmetadata*) list_find(segmento->metadatas,
				_direccion_de_metadata);
	}

	return ptr_seg_metadata;
}

void expandir_segmento(segment *segmento, int tam) {
	segmentmetadata *paux_seg_metadata = (segmentmetadata*) list_get(
			segmento->metadatas, (segmento->metadatas->elements_count - 1));
	heapmetadata *paux_metadata = paux_seg_metadata->metadata;
	int cant_pag = paginas_necesarias(
			tam - paux_metadata->bytes + sizeof(heapmetadata));
	agregar_paginas(segmento->tabla_de_paginas, cant_pag);
	paux_metadata->bytes += cant_pag * TAM_PAG;
}

bool metadatas_fusionables(segmentmetadata *paux_seg_metadata,
		segmentmetadata *ptr_seg_metadata) {
	heapmetadata *paux_metadata = paux_seg_metadata->metadata;
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
	if (ptr_seg_metadata->posicion_inicial + ptr_metadata->bytes
			+ sizeof(heapmetadata) == paux_seg_metadata->posicion_inicial
			&& !paux_metadata->ocupado)
		return true;
	if (paux_seg_metadata->posicion_inicial
			== ptr_seg_metadata->posicion_inicial
					- paux_seg_metadata->metadata->bytes
			&& !paux_metadata->ocupado)
		return true;
	return false;
}

segmentmetadata* buscar_metadata_para_anidar(t_list *metadatas,
		segmentmetadata *ptr_seg_metadata) {
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;

	bool _buddy_system_metadatas(void *element) {
		segmentmetadata *paux_seg_metadata = (segmentmetadata*) element;
		if (metadatas_fusionables(paux_seg_metadata, ptr_seg_metadata))
			return true;
		return false;
	}

	return list_find(metadatas, _buddy_system_metadatas);
}

int index_del_segment_metadata(segmentmetadata *ptr_seg_metadata,
		t_list *metadatas) {
	segment *ptr_seg_metadata_comparador;
	for (int index = 0; index < metadatas->elements_count; index++) {
		ptr_seg_metadata_comparador = list_get(metadatas, index);
		if (!memcmp(ptr_seg_metadata, ptr_seg_metadata_comparador,
				sizeof(segment))) { //Si son iguales devuelve 0
			return index;
		}
	}
	return -1;
}

void buddy_system(segmentmetadata *ptr_seg_metadata, t_list *metadatas) {
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
	segmentmetadata *ptr_seg_metadata_libre = buscar_metadata_para_anidar(
			metadatas, ptr_seg_metadata);
	if (ptr_seg_metadata_libre) {
		heapmetadata *ptr_metadata_libre = ptr_seg_metadata_libre->metadata;
		ptr_metadata->bytes += ptr_metadata_libre->bytes;
		free(ptr_metadata_libre);
		list_remove(metadatas,
				index_del_segment_metadata(ptr_seg_metadata_libre, metadatas));
	}
}

// Funcion para "debugger"
void mostrar_metadatas(t_list* metadatas_random) {
	void _mostrar_por_pantalla(void*element) {
		segmentmetadata *ptr_seg_metadata = (segmentmetadata*) element;
		heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
		printf(
				"-Metadata:\n     - Posicion inicial: %d\n     - Ocupado: %d \n     - Bytes: %d\n",
				ptr_seg_metadata->posicion_inicial, ptr_metadata->ocupado,
				ptr_metadata->bytes);
	}

	list_iterate(metadatas_random, &_mostrar_por_pantalla);
}

void mostrar_segmentos(t_list *tabla_de_segmentos) {
	void _mostrar_segmentos(void *element) {
		segment *ptr_segmento = (segment*) element;
		printf("\n\n-Segmento nro.%d\n -Cantidad de paginas del segmento: %d\n",
				posicion_en_tabla_segmento(ptr_segmento, tabla_de_segmentos),
				ptr_segmento->tabla_de_paginas->elements_count);
		mostrar_metadatas(ptr_segmento->metadatas);
	}
	list_iterate(tabla_de_segmentos, _mostrar_segmentos);
}

void mostrar_tabla_de_segmentos(t_list *tabla_de_segmentos) {
	void _mostrar_estructura_segmento(void *element) {
		segment *ptr_segmento = (segment*) element;
		printf(
				"Segmento nro. %d\n     -Base logica:%u\n     -Limite:%u\n     -Tam.%d\n",
				posicion_en_tabla_segmento(ptr_segmento, tabla_de_segmentos),
				ptr_segmento->base_logica, limite_segmento(ptr_segmento),
				ptr_segmento->tamanio);
	}
	list_iterate(tabla_de_segmentos, _mostrar_estructura_segmento);
}

bool direccion_pisa_alguna_metadata(segment *ptr_segmento,
		uint32_t direccion_pedida, int cantidad_de_bytes) {

	bool _coincide_con_metadata(void* element) {
		segmentmetadata *ptr_seg_metadata = ((segmentmetadata*) element);
		uint32_t direccion_seg_metadata = ptr_seg_metadata->posicion_inicial;
		bool condicionInicial = ((direccion_pedida >= direccion_seg_metadata)
				&& (direccion_pedida
						< (direccion_seg_metadata + sizeof(heapmetadata))));
		bool condicionFinal = (((direccion_pedida + cantidad_de_bytes)
				> direccion_seg_metadata)
				&& (direccion_seg_metadata > direccion_pedida));
		return condicionFinal || condicionFinal;
	}

	if (list_find(ptr_segmento->metadatas, _coincide_con_metadata))
		return true;

	return false;
}
