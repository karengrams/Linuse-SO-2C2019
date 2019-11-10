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


