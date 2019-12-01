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
		list_add(lista, pagina);
	}
	asignar_marcos(lista);
	return lista;
}

void agregar_paginas(t_list* tabla_de_paginas, int cantidadDePaginas) {
	page *pagina;
	for (int i = 0; i < cantidadDePaginas; i++) {
		pagina = crear_pagina();
		asignar_marco(pagina);
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

int posicion_en_tabla_paginas(page* elemento, t_list *tabla_de_paginas) {
	page *comparador;
	for (int index = 0; index < tabla_de_paginas->elements_count; index++) {
		comparador = list_get(tabla_de_paginas, index);
		if (!memcmp(elemento, comparador, sizeof(page))) { //Si son iguales devuelve 0
			return index;
		}
	}
	return -1;
}

void swap_pages(page* victima, page* paginaPedida){
	//datos de la victima
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
}

void traer_pagina(page* pagina){
	//cada vez que referencian
	//una pagina si no esta en memoria la buscamos
	//y cargamos, si esta en memoria seteamos el bit de uso

	if (!pagina->bit_presencia){
		page* victima = algoritmo_clock_modificado();
		swap_pages(victima, pagina);
	}
	pagina->bit_uso = true;

}

page* buscar_cero_cero(){
	for(int i=0; i<list_size(FRAMES_TABLE); i++){

		page* pagina = PAGINAS_EN_FRAMES[INDICE_ALGORITMO_CLOCK];

		if((pagina->bit_uso == 0) && (pagina->bit_modificado == 0)){
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

		if((pagina->bit_uso == 0)){
			incrementar_indice();
			return pagina;
		}
		pagina->bit_uso = 0; //lo seteamos en 0 y avanzamos
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

	while(!victima){

		victima = buscar_cero_cero();

		if(!victima){
			victima = buscar_cero_uno();
		}

	}

	return victima;
}

void escribir_en_archivo_swap(void *file, t_list *tabla_de_paginas, size_t tam){
	int offset = tam;
	int tam_archivo = strlen((char*)file);
	void *padding;
	bool archivo_completo=false;
	void _escribir_en_frame_de_swap (void *element){
		page *ptr_pagina = (page*)element;
		int pag_pos = posicion_en_tabla_paginas(ptr_pagina,tabla_de_paginas);
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
		asignar_marco_en_swap(pagina);
		list_add(tabla_de_paginas, pagina);
		escribir_pagina_extra_en_archivo_swap(pagina,tabla_de_paginas);
	}

}
