#include "frames.h"
#include <commons/memory.h>

void inicilizar_tabla_de_frames() {
	FRAMES_TABLE = list_create();
}

void dividir_memoria_en_frames(void * memoria, int pagetam, int memtam) {
	frame *frame_ptr;
	int cantidadDeFrames = memtam/pagetam;
	PAGINAS_EN_FRAMES = malloc(cantidadDeFrames*sizeof(page*));
	for (int i = 0; i < cantidadDeFrames; i++) {
		frame_ptr = (frame*) malloc(sizeof(frame));
		(*frame_ptr).memoria = memoria + i * pagetam;
		(*frame_ptr).nro_frame = i;
		list_add(FRAMES_TABLE, frame_ptr);
		PAGINAS_EN_FRAMES[i] = NULL; //YA INICIALIZO EL VECTOR EN NULL CON LA CANTIDAD DE ELEMENTOS
									 //IGUAL A LA CANTIDAD DE FRAMES
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

void escribir_metadata_en_frame(segment* ptr_segmento,segmentheapmetadata* paux_metadata_ocupado) {
	uint32_t direccionAbsoluta = paux_metadata_ocupado->posicion_inicial+ ptr_segmento->base_logica;
	int numeroPagina = numero_pagina(ptr_segmento, direccionAbsoluta);
	int desplazamiento = desplazamiento_en_pagina(ptr_segmento,direccionAbsoluta);
	char *ptr_loco = malloc(TAM_PAG);
	void *ptr_metadata = serializar_heap_metadata(paux_metadata_ocupado->metadata,sizeof(heapmetadata));

	if (TAM_PAG - desplazamiento >= sizeof(heapmetadata)) { //si entra copiamos solo en esa pagina
		page* pagina = (page*) list_get(ptr_segmento->tabla_de_paginas,numeroPagina);
		traer_pagina(pagina);
		frame* ptr_frame_aux = (frame*) pagina->frame; //Por alguna razon no me dejaba entrar al campo memoria si no hacia esto
		heapmetadata *ptr_metadata = paux_metadata_ocupado->metadata;
		sem_wait(&mutex_write_frame);
		memcpy(ptr_frame_aux->memoria+desplazamiento,ptr_metadata,sizeof(heapmetadata));
		sem_post(&mutex_write_frame);

	} else { //si no entra lo copiamos de a pedazos
		page* paginaUno = (page*) list_get(ptr_segmento->tabla_de_paginas,numeroPagina);
		page* paginaDos = (page*) list_get(ptr_segmento->tabla_de_paginas,numeroPagina + 1);
		traer_pagina(paginaUno);
		traer_pagina(paginaDos);
		frame* ptr_frame_aux_uno = (frame*)paginaUno->frame;
		frame* ptr_frame_aux_dos = (frame*)paginaDos->frame;
		int aCopiar = TAM_PAG - desplazamiento;
		sem_wait(&mutex_write_frame);
		memcpy(ptr_frame_aux_uno->memoria + desplazamiento,ptr_metadata, aCopiar);
		memcpy(ptr_frame_aux_dos->memoria,ptr_metadata+aCopiar,sizeof(heapmetadata)-aCopiar);
		sem_post(&mutex_write_frame);
	}

}

void* serializar_heap_metadata(heapmetadata* metadata, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(metadata->bytes), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(metadata->ocupado), sizeof(bool));
	desplazamiento+= sizeof(int);

	return magic;
}
