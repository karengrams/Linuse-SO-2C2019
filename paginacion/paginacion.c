#include "paginacion.h"

int INDICE_ALGORITMO_CLOCK = 0; //ptr para el algoritmo

page* crear_pagina() {
	page* pagina = malloc(sizeof(page));
	pagina->bit_presencia = 0;
	pagina->frame = NULL;
	return pagina;
}

t_list* crear_tabla_de_paginas(int tam) {
	int cantidadDePaginas = paginas_necesarias(tam);
	t_list *lista = list_create();
	page *pagina;

	for (int i = 0; i < cantidadDePaginas; i++) {
		pagina = crear_pagina();
		pagina->nro_pagina=i;
		list_add(lista, pagina);
	}
	return lista;
}

void agregar_paginas(t_list* tabla_de_paginas, int cantidadDePaginas,int index) {
	page *pagina;
	int index_pag=index;
	for (int i = 0; i < cantidadDePaginas; i++) {
		pagina = crear_pagina();
		pagina->nro_pagina=index_pag;
		asignar_marco(pagina);
		index_pag++;
		list_add(tabla_de_paginas, pagina);
	}
}

int paginas_necesarias(int valorPedido) {
	div_t aux = div(valorPedido, TAM_PAG);
	if (aux.rem == 0) {
		return aux.quot;
	} else {
		return aux.quot + 1;
	}
}

void asignar_marco(page* pag) {
	frame *marco_libre = obtener_marco_libre();
	sem_wait(&mutex_frames);
	if(!marco_libre){ //si no hay marcos libres buscamos en el swap
		asignar_marco_en_swap(pag);
	} else {
		bitarray_set_bit(BIT_ARRAY_FRAMES, (off_t) marco_libre->nro_frame);
		pag->frame = marco_libre;
		pag->bit_presencia = true;
		pag->nro_frame = marco_libre->nro_frame;
		pag->bit_uso = true;
		pag->bit_modificado = false;
	}

	PAGINAS_EN_FRAMES[marco_libre->nro_frame] = pag;
	sem_post(&mutex_frames);
}

void asignar_marco_en_swap(page* pag){
	int posicionLibre = posicion_libre_en_swap();
	bitarray_set_bit(BIT_ARRAY_SWAP, (off_t) posicionLibre);
	pag->frame = NULL;
	pag->bit_presencia = false;
	pag->nro_frame = posicionLibre;
	pag->bit_uso = false;
	pag->bit_modificado = false;
}


void swap_pages(page* victima, page* paginaPedida){
	//datos de la victima
	sem_wait(&binary_swap_pages);
	log_trace("se swapea la pagina #%d con la pagina #%d",paginaPedida->nro_pagina,victima->nro_pagina);
	int nroFrame = victima->nro_frame;
	frame* frameVictima = ((frame*)list_get(FRAMES_TABLE, nroFrame));

	int posicionEnSwap = paginaPedida->nro_frame*TAM_PAG;

	void* frameAReemplazar = frameVictima->memoria;
	void* bufferAux = malloc(TAM_PAG);

	memcpy(bufferAux, VIRTUAL_MEMORY+posicionEnSwap, TAM_PAG); //Swap mappeado como variable global por ahora
	memcpy(VIRTUAL_MEMORY+posicionEnSwap, frameAReemplazar, TAM_PAG);
	memcpy(frameAReemplazar, bufferAux, TAM_PAG);

	victima->bit_presencia = false;
	victima->frame = NULL;
	victima->nro_frame = paginaPedida->nro_frame;

	paginaPedida->bit_presencia = true;
	paginaPedida->frame = frameVictima;
	paginaPedida->nro_frame= nroFrame;
	paginaPedida->bit_uso = true;
	paginaPedida->bit_modificado = false;

	PAGINAS_EN_FRAMES[nroFrame] = paginaPedida; //cargamos la pagina pedida en el vector de paginas cargadas en memoria
	free(bufferAux);
	sem_post(&mutex_clock_mod);
}

void traer_pagina(page* pagina){
	//cada vez que referencian
	//una pagina si no esta en memoria la buscamos
	//y cargamos, si esta en memoria seteamos el bit de uso
	void *buffer=malloc(TAM_PAG);
	if (!pagina->bit_presencia){
		log_trace(logger_trace,"se produce un page fault (pagina #%d)",pagina->nro_pagina);
		frame *marco_libre = obtener_marco_libre();
		if(marco_libre){
			log_trace(logger_trace,"se procede a asignar el marco #%d a la pagina #%d",marco_libre->nro_frame,pagina->nro_pagina);
			memcpy(marco_libre->memoria, VIRTUAL_MEMORY+pagina->nro_frame*TAM_PAG, TAM_PAG); //Swap mappeado como variable global por ahora
			bitarray_clean_bit(BIT_ARRAY_SWAP,(off_t) pagina->nro_frame);
			bitarray_set_bit(BIT_ARRAY_FRAMES, (off_t) marco_libre->nro_frame);
			pagina->frame = marco_libre;
			pagina->bit_presencia = true;
			pagina->nro_frame = marco_libre->nro_frame;
			pagina->bit_uso = true;
			pagina->bit_modificado = false;
			PAGINAS_EN_FRAMES[marco_libre->nro_frame] = pagina;
		}else{
			page* victima = algoritmo_clock_modificado();
			swap_pages(victima, pagina);
		}
		sem_post(&mutex_frames);
	}
	pagina->bit_uso = true;
}

page* buscar_cero_cero(){
	for(int i=0; i<list_size(FRAMES_TABLE); i++){

		page* pagina = PAGINAS_EN_FRAMES[INDICE_ALGORITMO_CLOCK];

		if(pagina && (pagina->bit_uso == 0) && (pagina->bit_modificado == 0)){
			incrementar_indice();
			return pagina;
		}

		incrementar_indice();
	}
	return NULL;
}

page* buscar_cero_uno(){

	for(int i=0; i<list_size(FRAMES_TABLE); i++){

		page* pagina = PAGINAS_EN_FRAMES[INDICE_ALGORITMO_CLOCK];

		if(pagina && (pagina->bit_uso == 0)){
			incrementar_indice();
			return pagina;
		}else{
			pagina->bit_uso = 0; //lo seteamos en 0 y avanzamos
		}
		incrementar_indice();
	}
	return NULL;
}

void incrementar_indice(){
	if(INDICE_ALGORITMO_CLOCK == (list_size(FRAMES_TABLE)-1)){
		INDICE_ALGORITMO_CLOCK = 0;
	} else {
		INDICE_ALGORITMO_CLOCK ++;
	}
}

page* algoritmo_clock_modificado(){
	page* victima = NULL;
	log_trace(logger_trace,"se comienza a ejecutar el algoritmo clock modificado.");
	sem_wait(&mutex_clock_mod);
	while(!victima){

		victima = buscar_cero_cero();

		if(!victima){
			victima = buscar_cero_uno();
		}

	}
	log_trace(logger_trace,"se eligio como victima la pagina #%d cuyo frame es #%d.",victima->nro_pagina,victima->nro_frame);
	sem_post(&binary_swap_pages);
	return victima;
}

void escribir_en_archivo_swap(void *file, t_list *tabla_de_paginas, size_t tam_a_mappear,size_t tam_arch){
	int offset = tam_a_mappear;
	int tam_archivo = tam_arch;
	void *padding;
	bool archivo_completo=false;
	void _escribir_en_frame_de_swap (void *element){
		page *ptr_pagina = (page*)element;
		int pag_pos = ptr_pagina->nro_pagina;
		int posicion_en_swap = TAM_PAG * ptr_pagina->nro_frame;
		if(offset >0 && archivo_completo){
			padding = malloc(TAM_PAG);
			memset(padding,'\0',TAM_PAG);
			memcpy(VIRTUAL_MEMORY+posicion_en_swap,padding,TAM_PAG);
			offset -= TAM_PAG;
			free(padding);
		}
		if(tam_archivo>=TAM_PAG && offset > 0 && !archivo_completo){
			memcpy(VIRTUAL_MEMORY+posicion_en_swap,file+(pag_pos*TAM_PAG),TAM_PAG);
			offset -= TAM_PAG;
			tam_archivo-= TAM_PAG;
		}else if (offset > 0 && tam_archivo>0){
			memcpy(VIRTUAL_MEMORY+posicion_en_swap,file+(pag_pos*TAM_PAG),tam_archivo);
			padding = malloc(TAM_PAG-tam_archivo);
			memset(padding,'\0',TAM_PAG-tam_archivo);
			memcpy(VIRTUAL_MEMORY+posicion_en_swap+tam_archivo,padding,TAM_PAG-tam_archivo);
			offset-=TAM_PAG;
			tam_archivo-= TAM_PAG;
			archivo_completo=true;
			free(padding);
		}

	}
	list_iterate(tabla_de_paginas,_escribir_en_frame_de_swap);
}

void escribir_pagina_extra_en_archivo_swap(page*ptr_pagina,t_list *tabla_de_paginas){
	int posicion_en_swap = TAM_PAG * ptr_pagina->nro_frame;
	void *padding = malloc(TAM_PAG);
	memset(padding,'\0',TAM_PAG);
	memcpy(VIRTUAL_MEMORY+posicion_en_swap,padding,TAM_PAG);
}

void agregar_paginas_extras(t_list* tabla_de_paginas, int cantidadDePaginas){
	page *pagina;
	for (int i = 0; i < cantidadDePaginas; i++) {
		pagina = crear_pagina();
		sem_wait(&mutex_swap);
		asignar_marco_en_swap(pagina);
		sem_post(&mutex_swap);
		list_add(tabla_de_paginas, pagina);
		sem_wait(&mutex_swap_file);
		escribir_pagina_extra_en_archivo_swap(pagina,tabla_de_paginas);
		sem_post(&mutex_swap_file);
	}
}


void eliminar_pagina(void*element){
	page* ptr_pagina = (page*)element;
	if(ptr_pagina->bit_presencia)
		bitarray_clean_bit(BIT_ARRAY_FRAMES,ptr_pagina->nro_frame);
	else
		bitarray_clean_bit(BIT_ARRAY_SWAP,ptr_pagina->nro_frame);
	free(ptr_pagina);
}

