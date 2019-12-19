#include "segmentacion.h"

int minimo(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

segment* crear_segmento(segment_type tipo, int tam, t_list* tabla_de_segmentos) {
	segment *segmento_ptr;

	if(tipo == HEAP)
		segmento_ptr=crear_segmento_heap(tam,tabla_de_segmentos);
	else
		segmento_ptr=crear_segmento_map(tam,tabla_de_segmentos);

	return segmento_ptr;
}

uint32_t calculo_base_logica(segment *segmento_ptr, t_list *tabla_de_segmentos) {
	int pos_seg = segmento_ptr->nro_segmento;
	segment* segmento_anterior = (segment*) list_get(tabla_de_segmentos,pos_seg - 1);
	if (!segmento_anterior) {
		return 0; // Si no hay ningun segmento atras
	}
	return (*segmento_anterior).base_logica
			+ (*segmento_anterior).tabla_de_paginas->elements_count * TAM_PAG;
}

segment* buscar_segmento_heap_para_tam(t_list* tabla_de_segmentos, int tam) {

	bool _segmento_de_tipo_heap_que_tenga_espacio_suficiente(void*element) {
		segment *ptr_segmento=(segment*)element;
		if(ptr_segmento->tipo==HEAP)
			return segmento_de_tipo_heap_y_con_espacio(tam, element);
		else
			return false;
	}

	return (segment*) list_find(tabla_de_segmentos,_segmento_de_tipo_heap_que_tenga_espacio_suficiente);
}

segment* buscar_segmento_heap_expandible_para_tam(t_list* tabla_de_segmentos,int tam) {

	bool _segmento_de_tipo_heap_que_se_pueda_expandir(void*element) {
		return segmento_de_tipo_heap_y_expandible(tam, tabla_de_segmentos,element);
	}

	return (segment*) list_find(tabla_de_segmentos,_segmento_de_tipo_heap_que_se_pueda_expandir);

}

bool segmento_de_tipo_heap_y_expandible(int tam, t_list* tabla_de_segmentos, void*element) {
	segment *ptr_segmento = (segment*) element;
	return ptr_segmento->tipo == HEAP && segmento_puede_agrandarse(ptr_segmento, tam, tabla_de_segmentos);
}

bool segmento_de_tipo_heap_y_con_espacio(int tam, void*element) {
	segment *ptr_segmento = (segment*) element;

	bool _tiene_espacio_suficiente(void*element) {
		return tiene_espacio_suficiente(tam, element);
	}

	bool tiene_espacio = list_any_satisfy(ptr_segmento->metadatas,_tiene_espacio_suficiente);

	if (list_is_empty(ptr_segmento->metadatas))
		tiene_espacio = true;

	return tiene_espacio;
}

bool tiene_espacio_suficiente(int tam, void*element) {
	segmentheapmetadata *ptr_seg_metadata = (segmentheapmetadata*) element;
	return ((ptr_seg_metadata->metadata->bytes) >= (tam + sizeof(heapmetadata)) || (ptr_seg_metadata->metadata->bytes) >= tam) && !ptr_seg_metadata->metadata->ocupado;
}

uint32_t limite_segmento(segment* segmento) {
	return (segmento->base_logica+ list_size(segmento->tabla_de_paginas) * TAM_PAG) - 1;
}

bool segmento_puede_agrandarse(segment* segmento, int valorPedido,t_list*tabla_de_segmentos) {
	int pos_seg = segmento->nro_segmento;
	int paginasNecesarias;
	segment *siguiente = (segment*) list_get(tabla_de_segmentos, (pos_seg + 1));

	segmentheapmetadata *ptr_seg_metadata = (segmentheapmetadata*) list_get(segmento->metadatas, segmento->metadatas->elements_count - 1);
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;

	if(!ptr_metadata->ocupado) // Si el ultimo metadata es de ocupado, entonces no se deberia restar. Si es de libre, si.
		paginasNecesarias = paginas_necesarias(valorPedido + sizeof(heapmetadata) - ptr_metadata->bytes);
	else
		paginasNecesarias = paginas_necesarias(valorPedido + sizeof(heapmetadata));


	if (!siguiente) {
		return true; //Es el ultimo elemento de la lista
	}
	if (((segmento->base_logica + (segmento->tabla_de_paginas->elements_count + paginasNecesarias) * TAM_PAG) - 1) < siguiente->base_logica)
		return true;

	return false;
}

int numero_pagina(segment* segmento, uint32_t direccion) {
	int desplazamientoEnSegmento = direccion - segmento->base_logica;
	return div(desplazamientoEnSegmento, TAM_PAG).quot;
}

int desplazamiento_en_pagina(segment* segmento, uint32_t direccion) {
	int desplazamiento = (int)div(direccion - segmento->base_logica,TAM_PAG).rem;
	return desplazamiento;
}

segment* buscar_segmento_dada_una_direccion(uint32_t direccion,t_list*tabla_de_segmentos) {
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
	}else {
		for (int i = 0; i < paux_metadatas->elements_count; i++){
			segmentheapmetadata* ptr_seg_metadata = (segmentheapmetadata*) list_get(paux_metadatas, i);
			if (
					(!ptr_seg_metadata->metadata->ocupado) && (ptr_seg_metadata->metadata->bytes>= (tam)
							|| ptr_seg_metadata->metadata->bytes == tam)
			)
				direccion = ptr_seg_metadata->posicion_inicial;
		}
	}
	return direccion;
}

segmentheapmetadata *buscar_metadata_para_liberar(uint32_t direccion, segment *segmento){
	bool _direccion_de_metadata(void* element) {
		segmentheapmetadata *ptr_seg_metadata = ((segmentheapmetadata*) element);
		uint32_t direccion_seg_metadata = sizeof(heapmetadata) + ptr_seg_metadata->posicion_inicial;
		return direccion_seg_metadata == direccion;
	}
	return (segmentheapmetadata*) list_find(segmento->metadatas,_direccion_de_metadata);
}

segmentheapmetadata *buscar_metadata_de_segmento_segun_dir(uint32_t dir,segment*segmento){
	bool _metadata_dada_una_direccion(void*element){
		segmentheapmetadata* ptr_seg_metadata = (segmentheapmetadata*)element;
		heapmetadata* ptr_metadata = ptr_seg_metadata->metadata;
		return ptr_seg_metadata->posicion_inicial+sizeof(heapmetadata)<=dir || ptr_seg_metadata->posicion_inicial+sizeof(heapmetadata)+ptr_metadata->bytes >dir;
	}

	return (segmentheapmetadata*)list_find(segmento->metadatas,_metadata_dada_una_direccion);
}

segmentheapmetadata* buscar_metadata_de_segmento_segun(uint32_t offset, segment* segmento) {
	bool _direccion_de_metadata(void* element) {
		return ((segmentheapmetadata*) element)->posicion_inicial == offset;
	}
	segmentheapmetadata *ptr_seg_metadata;

	if (list_is_empty(segmento->metadatas)) {
		ptr_seg_metadata = (segmentheapmetadata*) malloc(sizeof(segmentheapmetadata));
		heapmetadata *ptr_metadata = (heapmetadata*) malloc(sizeof(heapmetadata));
		ptr_metadata->bytes = (segmento->tabla_de_paginas->elements_count* TAM_PAG) - sizeof(heapmetadata);
		ptr_metadata->ocupado = false;
		ptr_seg_metadata->metadata = ptr_metadata;
		ptr_seg_metadata->posicion_inicial = offset;
		list_add(segmento->metadatas, ptr_seg_metadata);
	} else {
		ptr_seg_metadata = (segmentheapmetadata*) list_find(segmento->metadatas,
				_direccion_de_metadata);
	}

	return ptr_seg_metadata;
}

void expandir_segmento(segment *segmento,int tam){
	segmentheapmetadata *paux_seg_metadata = (segmentheapmetadata*)list_get(segmento->metadatas,(segmento->metadatas->elements_count-1));
	heapmetadata *paux_metadata = paux_seg_metadata->metadata;
	int cant_pag;
	if(!paux_metadata->ocupado)
		cant_pag = paginas_necesarias(tam-paux_metadata->bytes+sizeof(heapmetadata));
	else
		cant_pag = paginas_necesarias(tam+sizeof(heapmetadata));

	agregar_paginas(segmento->tabla_de_paginas,cant_pag,segmento->tabla_de_paginas->elements_count);
	if(paux_metadata->ocupado){ //El ultimo metadata es de ocupado. Se deberia agregar uno nuevo de libre
		segmentheapmetadata *seg_libre = (segmentheapmetadata*)malloc(sizeof(segmentheapmetadata));
		heapmetadata *libre = (heapmetadata*) malloc(sizeof(heapmetadata));
		(*libre).ocupado=false;
		(*libre).bytes=cant_pag*TAM_PAG-sizeof(heapmetadata);
		(*seg_libre).metadata=libre;
		(*seg_libre).posicion_inicial=paux_seg_metadata->posicion_inicial+paux_metadata->bytes+sizeof(heapmetadata);
		list_add(segmento->metadatas,seg_libre);
		escribir_metadata_en_frame(segmento,paux_seg_metadata);
	}else
		paux_metadata->bytes+=cant_pag*TAM_PAG;
}

bool metadatas_fusionables(segmentheapmetadata *otro_ptr_seg_metadata, segmentheapmetadata *ptr_seg_metadata) {
	heapmetadata *otro_ptr_metadata = otro_ptr_seg_metadata->metadata;
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
	if (otro_ptr_seg_metadata->posicion_inicial + otro_ptr_metadata->bytes+ sizeof(heapmetadata) == ptr_seg_metadata->posicion_inicial && !otro_ptr_metadata->ocupado)
		return true;
	if (otro_ptr_seg_metadata->posicion_inicial == ptr_seg_metadata->posicion_inicial + sizeof(heapmetadata) +  ptr_metadata->bytes && !otro_ptr_metadata->ocupado)
		return true;
	return false;
}

segmentheapmetadata* buscar_metadata_para_anidar(t_list *metadatas,segmentheapmetadata *ptr_seg_metadata) {

	bool _buddy_system_metadatas(void *element) {
		segmentheapmetadata *otro_ptr_seg_metadata = (segmentheapmetadata*) element;
		if (metadatas_fusionables(otro_ptr_seg_metadata, ptr_seg_metadata))
			return true;
		return false;
	}

	return list_find(metadatas, _buddy_system_metadatas);
}

int index_del_segment_metadata(segmentheapmetadata *ptr_seg_metadata,t_list *metadatas) {
	segmentheapmetadata *ptr_seg_metadata_comparador;
	for (int index = 0; index < metadatas->elements_count; index++) {
		ptr_seg_metadata_comparador = list_get(metadatas, index);
		if (ptr_seg_metadata->posicion_inicial==ptr_seg_metadata_comparador->posicion_inicial) { //Si son iguales devuelve 0
			return index;
		}
	}
	return -1;
}

void buddy_system(segmentheapmetadata *ptr_seg_metadata, t_list *metadatas) {
	heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
	while(buscar_metadata_para_anidar(metadatas, ptr_seg_metadata)){
		segmentheapmetadata *ptr_seg_metadata_libre = buscar_metadata_para_anidar(metadatas, ptr_seg_metadata);
		heapmetadata *ptr_metadata_libre = ptr_seg_metadata_libre->metadata;
		ptr_metadata->bytes += ptr_metadata_libre->bytes+sizeof(heapmetadata);
		if(ptr_seg_metadata_libre->posicion_inicial<ptr_seg_metadata->posicion_inicial){
			ptr_seg_metadata->posicion_inicial=ptr_seg_metadata_libre->posicion_inicial;
		}
		list_remove(metadatas, index_del_segment_metadata(ptr_seg_metadata_libre, metadatas));
		free(ptr_metadata_libre);

	}
}

bool direccion_pisa_alguna_metadata(segment *ptr_segmento,uint32_t direccion_pedida, int cantidad_de_bytes) {

	bool _coincide_con_metadata(void* element) {
		segmentheapmetadata *ptr_seg_metadata = ((segmentheapmetadata*) element);
		uint32_t direccion_seg_metadata = ptr_seg_metadata->posicion_inicial;
		bool condicionInicial = ((direccion_pedida >= direccion_seg_metadata)
				&& (direccion_pedida
						< (direccion_seg_metadata + sizeof(heapmetadata))));
		bool condicionFinal = (((direccion_pedida + cantidad_de_bytes)
				> direccion_seg_metadata)
				&& (direccion_seg_metadata > direccion_pedida));
		return condicionInicial || condicionFinal;
	}

	if (list_find(ptr_segmento->metadatas, _coincide_con_metadata))
		return true;

	return false;
}

segment* crear_segmento_heap(int tam, t_list* tabla_de_segmentos){
	segment *segmento_ptr = (segment*) malloc(sizeof(segment));
	(*segmento_ptr).nro_segmento=tabla_de_segmentos->elements_count;
	list_add(tabla_de_segmentos, segmento_ptr);
	(*segmento_ptr).tipo = HEAP;
	(*segmento_ptr).metadatas = list_create();
	(*segmento_ptr).tamanio=0;
	(*segmento_ptr).base_logica = calculo_base_logica(segmento_ptr,tabla_de_segmentos);
	(*segmento_ptr).tabla_de_paginas = crear_tabla_de_paginas(tam);
	asignar_marcos((*segmento_ptr).tabla_de_paginas);
	return segmento_ptr;
}

segment* crear_segmento_map(int tam, t_list* tabla_de_segmentos) {
	segment *segmento_ptr = (segment*) malloc(sizeof(segment));
	(*segmento_ptr).nro_segmento=tabla_de_segmentos->elements_count;
	list_add(tabla_de_segmentos, segmento_ptr);
	(*segmento_ptr).tamanio = tam;
	(*segmento_ptr).metadatas = list_create();
	(*segmento_ptr).base_logica = calculo_base_logica(segmento_ptr,tabla_de_segmentos);
	(*segmento_ptr).tipo = MMAP;
	return segmento_ptr;
}


void recalcular_bases_logicas_de_segmentos(t_list *tabla_de_segmentos){
	int index=0;
	void _recalcular_base_logica(void*element){
		segment* ptr_segmento = (segment*)element;
		ptr_segmento->nro_segmento=index;
		ptr_segmento->base_logica=calculo_base_logica(ptr_segmento,tabla_de_segmentos);
		index++;
	}
	list_iterate(tabla_de_segmentos,_recalcular_base_logica);
}


void liberar_recursos_segmento_heap(segment*ptr_segmento,t_proceso* proceso){

	void _eliminar_metadatas(void*element){
		eliminar_metadatas(element,ptr_segmento);
	}

	void _eliminar_paginas(void*element){
		eliminar_pagina(element);
	}

	list_destroy_and_destroy_elements(ptr_segmento->tabla_de_paginas,_eliminar_paginas);
	list_destroy_and_destroy_elements(ptr_segmento->metadatas,_eliminar_metadatas);

}

void eliminar_metadatas(void *element,segment*ptr_segmento){
	if(ptr_segmento->tipo==HEAP){
			segmentheapmetadata *ptr_heap_metadata = (segmentheapmetadata*)element;
			free(ptr_heap_metadata->metadata);
			free(ptr_heap_metadata);
		}else{
			segmentmmapmetadata *ptr_map_metadata = (segmentmmapmetadata*)element;
			free(ptr_map_metadata->path);
			free(ptr_map_metadata);
		}
}

void liberar_recursos_segmento_map(segment *ptr_segmento,t_proceso*proceso){
	segmentmmapmetadata *ptr_metadata = (segmentmmapmetadata*)list_get(ptr_segmento->metadatas,0);
	mapped_file *ptr_mapped_metadata = buscar_archivo_abierto(ptr_metadata->path);

	bool _mismo_id(void*element){
		t_proceso *otro_proceso = (t_proceso*) element;
		return otro_proceso->id == proceso->id;
	}

	void _eliminar_paginas_compartidas(void*element){
		page * ptr_pagina = (page*)element;
		if(ptr_pagina->nro_frame<list_size(ptr_mapped_metadata->paginas_min_asignadas))
			list_remove(ptr_segmento->tabla_de_paginas,0);
	}

	void _eliminar_metadatas(void*element){
		eliminar_metadatas(element,ptr_segmento);
	}

	void _eliminar_paginas(void*element){
		eliminar_pagina(element);
	}

	list_remove(proceso->tablaDeSegmentos,ptr_segmento->nro_segmento);
	recalcular_bases_logicas_de_segmentos(proceso->tablaDeSegmentos);

	sem_wait(&mutex_shared_files);
	list_remove_by_condition(ptr_mapped_metadata->procesos,_mismo_id);
	list_iterate(ptr_segmento->tabla_de_paginas,_eliminar_paginas_compartidas);

	if(!ptr_mapped_metadata->procesos->elements_count && ptr_mapped_metadata->flag==MAP_SHARED){

		munmap(ptr_mapped_metadata->file,ptr_mapped_metadata->tam_archivo);
		list_destroy_and_destroy_elements(ptr_mapped_metadata->paginas_min_asignadas,_eliminar_paginas);
		list_destroy(ptr_mapped_metadata->procesos);
		free(ptr_mapped_metadata->path);
		printf("%d\n",MAPPED_FILES->elements_count);
		sleep(50);
		list_remove(MAPPED_FILES,ptr_mapped_metadata->nro_file);
		update_file_number();
		free(ptr_mapped_metadata);

	}else if(ptr_mapped_metadata && ptr_mapped_metadata->flag==MAP_PRIVATE){

		munmap(ptr_mapped_metadata->file,ptr_metadata->tam_mappeado);
		free(ptr_mapped_metadata->path);
		list_destroy(ptr_mapped_metadata->procesos);
		list_destroy(ptr_mapped_metadata->paginas_min_asignadas);
		list_remove(MAPPED_FILES,ptr_mapped_metadata->nro_file);
		update_file_number();
		free(ptr_mapped_metadata);
	}

	sem_post(&mutex_shared_files);
	list_destroy_and_destroy_elements(ptr_segmento->tabla_de_paginas,_eliminar_paginas);
	list_destroy_and_destroy_elements(ptr_segmento->metadatas,_eliminar_metadatas);
}



void liberar_recursos_del_segmento(segment*ptr_segmento,t_proceso* proceso){
	if(ptr_segmento->tipo==HEAP)
		liberar_recursos_segmento_heap(ptr_segmento,proceso);
	else
		liberar_recursos_segmento_map(ptr_segmento,proceso);
	free(ptr_segmento);
}

void liberar_tabla_de_segmentos(t_proceso *proceso){

	void _liberar_segmento(void*element){
		segment*ptr_segmento = (segment*)element;
		liberar_recursos_del_segmento(ptr_segmento,proceso);
	}

	list_destroy_and_destroy_elements(proceso->tablaDeSegmentos,&_liberar_segmento);
}

void eliminar_segmento_de_tabla(t_proceso*proceso,segment*segmento){
	int index = 0;
	void _liberar_recursos_del_segmento(void *element){
		segment *segmento = (segment*)element;
		liberar_recursos_del_segmento(segmento,proceso);
	}
	list_remove_and_destroy_element(proceso->tablaDeSegmentos,segmento->nro_segmento,_liberar_recursos_del_segmento);

	void _recalcular_nro_de_segmentos(void*element){
		segment *otro_segmento = (segment*)element;
		otro_segmento->nro_segmento=index;
		index++;
	}

	list_iterate(proceso->tablaDeSegmentos,_recalcular_nro_de_segmentos);

}

mapped_file* buscar_archivo_abierto(char*path){
	bool _archivo_fue_abierto(void *element){
		mapped_file *ptr_mapped_file = (mapped_file*)element;
		return !strcmp(path,ptr_mapped_file->path);
	}
	return (mapped_file*)list_find(MAPPED_FILES,_archivo_fue_abierto);
}

void update_file_number(){
	int index = 0;
	void _update_file_number(void*element){
		mapped_file*ptr_mapped_file=(mapped_file*)element;
		ptr_mapped_file->nro_file=0;
		index++;
	}

	list_iterate(MAPPED_FILES,_update_file_number);
}

int espacio_libre(segment*ptr_segmento){
	int free = 0;
	if(ptr_segmento->tipo==HEAP){
		void _espacio_total_libre(void*element){
			segmentheapmetadata *ptr_seg_metadata = (segmentheapmetadata*)element;
			heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
			if(!ptr_metadata->ocupado)
				free+=ptr_metadata->bytes;
		}
		list_iterate(ptr_segmento->metadatas,_espacio_total_libre);
		return free;
	}else
		return -1;
}


void escribir_metadata_en_frame(segment* ptr_segmento,segmentheapmetadata* paux_metadata_ocupado) {
	uint32_t direccionAbsoluta = paux_metadata_ocupado->posicion_inicial+ ptr_segmento->base_logica;
	int numeroPagina = numero_pagina(ptr_segmento, direccionAbsoluta);
	int desplazamiento = desplazamiento_en_pagina(ptr_segmento,direccionAbsoluta);
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

