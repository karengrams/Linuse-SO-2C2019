/*
 * frames.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include "frames.h"

void inicilizar_tabla_de_frames(){
	FRAMES_TABLE=list_create();
}

void dividir_memoria_en_frames(int pagetam, int memtam){
	metadata *metadata_ptr;
	frame *frame_ptr;
	for(int i=0;i<memtam/pagetam;i++){
		frame_ptr = (frame*)malloc(sizeof(frame));
		metadata_ptr = (metadata*)malloc(sizeof(metadata));
		(*metadata_ptr).bytes=pagetam;
		(*metadata_ptr).ocupado=false;
		(*frame_ptr).nro_frame=i;
		(*frame_ptr).usado=false;
		list_add(FRAMES_TABLE,frame_ptr);
		(*frame_ptr).metadatas=list_create();
		list_add(frame_ptr->metadatas,metadata_ptr);
	}
}

void inicializar_bitmap(){
	int bytes;
	int cantidadDeMarcos = list_size(FRAMES_TABLE);
	div_t aux = div(cantidadDeMarcos, 8);

	if (aux.rem == 0){
		bytes = aux.quot;
	}else {
		bytes = aux.quot + 1;
	}
	char *punteroABits = (char*)malloc(bytes+1);
	BIT_ARRAY_FRAMES = bitarray_create_with_mode(punteroABits, (size_t)bytes, LSB_FIRST);
}

frame* obtener_marco_libre(){
	bool _es_un_marco_libre(void *elemento) {
		int nro_de_frame=((frame*)elemento)->nro_frame;
		return !bitarray_test_bit(BIT_ARRAY_FRAMES,nro_de_frame);
	}
	return (frame*)list_find(FRAMES_TABLE,&_es_un_marco_libre);

}

void asignar_marcos(t_list* tabla_de_pags){
	void _asignar_marco(void *elemento){
		asignar_marco((page*)elemento);
	}
	list_iterate(tabla_de_pags,&_asignar_marco);
}

void asignar_marco(page *pag){
	frame *marco_libre=obtener_marco_libre();
	bitarray_set_bit(BIT_ARRAY_FRAMES,(off_t)marco_libre->nro_frame);
	pag->frame=marco_libre;
}
