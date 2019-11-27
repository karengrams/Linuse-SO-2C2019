#include "paginacion.h"

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

	if(!marco_libre){
		asignar_marco_en_swap(pag);
	} else {
		bitarray_set_bit(BIT_ARRAY_FRAMES, (off_t) marco_libre->nro_frame);
		pag->frame = marco_libre;
		pag->bit_presencia = true;
		pag->nro_frame = marco_libre->nro_frame;
		pag->bit_uso = true;
		pag->bit_modificado = false;
	}
}

//void cambiar_bit_de_prescencia(t_list *pages_table){
//
//	void _cambiar_bit_de_paginas(void*element){
//		page *ptr_page = (page*) element;
//		bool estado_anterior = ptr_page->bit_presencia;
//		ptr_page->bit_presencia=!estado_anterior;
//	}
//
//	list_iterate(pages_table,_cambiar_bit_de_paginas);
//}

int posicion_en_tabla_paginas(page* elemento, t_list *tabla_de_paginas) {
	segment *comparador;
	for (int index = 0; index < tabla_de_paginas->elements_count; index++) {
		comparador = list_get(tabla_de_paginas, index);
		if (!memcmp(elemento, comparador, sizeof(segment))) { //Si son iguales devuelve 0
			return index;
		}
	}
	return -1;
}

/*void swap_pages(page* victima, page* paginaPedida){
	//datos de la victima
	int nroFrame = victima->nro_frame
	frame* frameVictima = ((frame*)list_get(FRAMES_TABLE, nroFrame));

	int posicionEnSwap = paginaPedida->nroFrame*TAM_PAG;

	void* frameAReemplazar = frameVictima->memoria;
	void* buffer = malloc(TAM_PAG);

	fseek(archivoSwap,posicionEnSwap,SEEK_SET); //archivo swap como variable global?
	fread(buffer, TAM_PAG,1 , archivoSwap);
	fseek(archivoSwap,posicionEnSwap,SEEK_SET);
	fwrite(frameAReemplazar,TAM_PAG,1,archivoSwap);
	memcpy(frameAReemplazar, buffer, TAM_PAG);

	victima->bit_presencia = false;
	victima->frame = NULL;
	victima->nro_frame = paginaPedida->nro_frame

	paginaPedida->bit_presencia = true;
	paginaPedida->frame = frameVictima;
	paginaPedida->nro_frame= nroFrame;
	paginaPedida->bit_uso = true;
	paginaPedida->bit_modificado = false;

}*/

