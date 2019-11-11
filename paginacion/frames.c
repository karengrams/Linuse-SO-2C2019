/*
 * frames.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include "frames.h"
#include "paginacion.h"

void inicilizar_tabla_de_frames() {
	FRAMES_TABLE = list_create();
}

void dividir_memoria_en_frames(void * memoria ,int pagetam, int memtam) {
	heapmetadata *metadata_ptr;
	frame *frame_ptr;
	for (int i = 0; i < memtam / pagetam; i++) {
		frame_ptr = (frame*) malloc(sizeof(frame));
		(*frame_ptr).memoria=memoria+i*pagetam;
		(*frame_ptr).nro_frame = i;
		list_add(FRAMES_TABLE, frame_ptr);
	}
}

void inicializar_bitmap() {
	int bytes;
	int cantidadDeMarcos = list_size(FRAMES_TABLE);
	div_t aux = div(cantidadDeMarcos, 8);

	if (aux.rem == 0) {
		bytes = aux.quot;
	} else {
		bytes = aux.quot + 1;
	}
	char *punteroABits = (char*) malloc(bytes + 1);
	BIT_ARRAY_FRAMES = bitarray_create_with_mode(punteroABits, (size_t) bytes,
			LSB_FIRST);
}

frame* obtener_marco_libre() {
	bool _es_un_marco_libre(void *elemento) {
		int nro_de_frame = ((frame*) elemento)->nro_frame;
		return !bitarray_test_bit(BIT_ARRAY_FRAMES, nro_de_frame);
	}
	return (frame*) list_find(FRAMES_TABLE, &_es_un_marco_libre);

}

void asignar_marcos(t_list* tabla_de_pags) {
	void _asignar_marco(void *elemento) {
		asignar_marco((page*) elemento);
	}
	list_iterate(tabla_de_pags, &_asignar_marco);
}

escribir_metadata_en_frame(segment* ptr_segmento, segmentmetadata paux_metadata_ocupado){
	uint32_t direccionAbsoluta = paux_metadata_ocupado->posicion_inicial+ptr_segmento->base_logica;
	int numeroPagina = numero_pagina(ptr_segmento, direccionAbsoluta);
	int desplazamiento = desplazamiento_en_pagina(ptr_segmento, direccionAbsoluta);

	if(TAM_PAG - desplazamiento >= sizeof(metadata)){ //si entra copiamos solo en esa pagina
		page* pagina = (page*)list_get(ptr_segmento->tabla_de_paginas, numeroPagina);
		frame* ptr_frame_aux = pagina->frame; //Por alguna razon no me dejaba entrar al campo memoria si no hacia esto

		memcpy(ptr_frame_aux->memoria+desplazamiento, paux_metadata_ocupado->metadata, sizeof(metadata));

			} else { //si no entra lo copiamos de a pedazos
		page* paginaUno = (page*)list_get(ptr_segmento->tabla_de_paginas, numeroPagina);
		page* paginaDos = (page*)list_get(ptr_segmento->tabla_de_paginas, numeroPagina+1);
		frame* ptr_frame_aux_uno = paginaUno->frame;
		frame* ptr_frame_aux_dos = paginaDos->frame;
		int aCopiar = TAM_PAG - desplazamiento;

		memcpy(ptr_frame_aux_uno->memoria+desplazamiento, paux_metadata_ocupado->metadata, aCopiar);
		memcpy(ptr_frame_aux_dos->memoria, paux_metadata_ocupado->metadata+aCopiar, sizeof(metadata)-aCopiar);
		}

}
