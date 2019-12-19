#include "frames.h"
#include <commons/memory.h>

void inicilizar_tabla_de_frames() {
	FRAMES_TABLE = list_create();
}

void inicializar_tabla_de_clock(){
	PAGINAS_EN_FRAMES=list_create();
}

void dividir_memoria_en_frames(void * memoria, int pagetam, int memtam) {
	frame *frame_ptr;
	int cantidadDeFrames = div(memtam,pagetam).quot;
	PAGINAS_EN_FRAMES = malloc(cantidadDeFrames*sizeof(page*));
	for (int i = 0; i < cantidadDeFrames; i++) {
		frame_ptr = (frame*) malloc(sizeof(frame));
		(*frame_ptr).memoria = memoria + i * pagetam;
		(*frame_ptr).nro_frame = i;
		list_add(FRAMES_TABLE, frame_ptr);
	}
}

void inicializar_tabla_para_clock(int cantidad_de_marcos){
	for(int i=0;i<cantidad_de_marcos;i++){
		pages_in_frames*ptr_pag_in_frame = (pages_in_frames*)malloc(sizeof(pages_in_frames));
		ptr_pag_in_frame->ptr_frame=(frame*)list_get(FRAMES_TABLE,i);
		list_add(PAGINAS_EN_FRAMES,ptr_pag_in_frame);
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
	char *punteroABits = (char*) malloc(bytes);
	BIT_ARRAY_FRAMES = bitarray_create_with_mode(punteroABits, (size_t) bytes,LSB_FIRST);
	for(int i=0;i<cantidadDeMarcos;i++){
		bitarray_clean_bit(BIT_ARRAY_FRAMES,i);
	}
}

frame* obtener_marco_libre() {
	bool _es_un_marco_libre(void *elemento) {
		int nro_de_frame = ((frame*) elemento)->nro_frame;
		int bit = bitarray_test_bit(BIT_ARRAY_FRAMES, nro_de_frame);
		return !bit;
	}
	return(frame*) list_find(FRAMES_TABLE,_es_un_marco_libre);
}

void asignar_marcos(t_list* tabla_de_pags) {
	void _asignar_marco(void *elemento) {
		asignar_marco((page*) elemento);
	}
	list_iterate(tabla_de_pags, &_asignar_marco);
}

void asignar_marcos_swap(t_list* tabla_de_pags) {
	void _asignar_marco_swap(void *elemento) {
		asignar_marco_en_swap((page*) elemento);
	}
	list_iterate(tabla_de_pags, &_asignar_marco_swap);
}
