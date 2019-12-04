#include "muse.h"

#define ERROR -1;

void inicializar_tabla_procesos(){
	PROCESS_TABLE = list_create();
}

void inicializar_tabla_archivos_compartidos(){
	MAPPED_FILES = list_create();
}

void inicializar_memoria_virtual(int tam){
	crear_archivo_swap(tam);
}

t_config* leer_config(){
	return config_create("muse.config");
}

int leer_del_config(char* valor, t_config* archivo_config) {
	return config_get_int_value(archivo_config, valor);
}

void liberacion_de_recursos(int num){

	printf("Liberando recursos...\n");

	void _liberar_proceso(void*element){
		t_proceso *ptr_proceso = (t_proceso*) element;
		museclose(ptr_proceso);
	}

	void _liberar_frame(void *elemento) {
		frame *marco = (frame*) elemento;
		free(marco);
	}

	void _liberar_archivos_mappeados(void*element){
		mapped_file *file = (mapped_file*)element;
		munmap(file->file,file->tam_archivo);
		free(file->path);
		list_destroy(file->procesos);
		list_destroy(file->paginas_min_asignadas);
		free(file);
	}

	list_destroy_and_destroy_elements(PROCESS_TABLE,&_liberar_proceso);
	list_destroy_and_destroy_elements(FRAMES_TABLE, &_liberar_frame);
	list_destroy_and_destroy_elements(MAPPED_FILES, &_liberar_archivos_mappeados);
	munmap(VIRTUAL_MEMORY,leer_del_config("SWAP_SIZE",config));
	free(BIT_ARRAY_FRAMES->bitarray);
	free(BIT_ARRAY_SWAP->bitarray);
	free(PAGINAS_EN_FRAMES);
	free(memoria);
	bitarray_destroy(BIT_ARRAY_FRAMES);
	bitarray_destroy(BIT_ARRAY_SWAP);
	config_destroy(config);
	printf("Recursos liberados!\n");
	raise(SIGTERM);
}

int museinit(t_proceso* cliente_a_atender, char* ipCliente, int id){
	if (cliente_a_atender != NULL) {
		return ERROR; //YA EXISTE EN NUESTRA TABLA ERROR
	} else {
		t_proceso* procesoNuevo = crear_proceso(id, ipCliente);
		list_add(PROCESS_TABLE, procesoNuevo); //Si no existe lo creamos y agregamos
		return 0;
	}
}

void museclose(t_proceso* proceso){
	liberar_tabla_de_segmentos(proceso->tablaDeSegmentos);
	bool _mismo_id (void*element){
		t_proceso *otroproceso = (t_proceso*)element;
		return (otroproceso->id == proceso->id);
	}
	list_remove_by_condition(PROCESS_TABLE,_mismo_id);
	free(proceso->ip);
	free(proceso);
}

uint32_t musealloc(t_proceso* proceso,int tam){
	uint32_t offset;
	t_list *tabla_de_segmento = proceso->tablaDeSegmentos;
	segment *ptr_segmento = buscar_segmento_heap_para_tam(tabla_de_segmento,tam); // Busca segmento de tipo heap con espacio
	if(!ptr_segmento){
		ptr_segmento= buscar_segmento_heap_expandible_para_tam(tabla_de_segmento,tam);
		if(ptr_segmento)
			expandir_segmento(ptr_segmento,tam);

		else
			ptr_segmento=crear_segmento(HEAP,tam+sizeof(heapmetadata),tabla_de_segmento);

	}
	offset = obtener_offset_para_tam(ptr_segmento,tam);

	segmentheapmetadata *paux_seg_metadata_ocupado=buscar_metadata_de_segmento_segun(offset,ptr_segmento);
	heapmetadata *paux_metadata_ocupado = paux_seg_metadata_ocupado->metadata;

	if(paux_metadata_ocupado->bytes>tam+sizeof(heapmetadata)){ // Si los bytes que tenia libre antes de modificarlo, es mayor a lo pedido +
		// el tam. del heapmetadata, deberia ir el metadata de libre
		segmentheapmetadata *paux_seg_metadata_libre= (segmentheapmetadata*)malloc(sizeof(segmentheapmetadata));
		heapmetadata *paux_metadata_libre = (heapmetadata*) malloc(sizeof(heapmetadata));
		paux_metadata_libre->bytes=paux_metadata_ocupado->bytes-tam-sizeof(heapmetadata);
		paux_metadata_libre->ocupado=false;
		paux_seg_metadata_libre->posicion_inicial=offset+sizeof(heapmetadata)+tam;
		paux_seg_metadata_libre->metadata=paux_metadata_libre;
		list_add(ptr_segmento->metadatas,paux_seg_metadata_libre);
		escribir_metadata_en_frame(ptr_segmento, paux_seg_metadata_libre);
	}

	paux_metadata_ocupado->bytes=tam;
	paux_metadata_ocupado->ocupado=true;
	ptr_segmento->tamanio+=tam;
	escribir_metadata_en_frame(ptr_segmento, paux_seg_metadata_ocupado);

	proceso->totalMemoriaPedida += tam;

	return ptr_segmento->base_logica+offset+sizeof(heapmetadata);
}

void musefree(t_proceso *proceso, uint32_t direccion) {
	segment *ptr_segmento = buscar_segmento_dada_una_direccion(direccion,proceso->tablaDeSegmentos);
	if(ptr_segmento && ptr_segmento->tipo==HEAP){
		segmentheapmetadata *ptr_seg_metadata = buscar_metadata_para_liberar(direccion - ptr_segmento->base_logica, ptr_segmento);
		if (ptr_seg_metadata) {
			proceso->totalMemoriaLiberada =+ ptr_seg_metadata->metadata->bytes; //sumo la cant de bytes liberados
			heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
			ptr_metadata->ocupado = false;
			buddy_system(ptr_seg_metadata, ptr_segmento->metadatas);
			escribir_metadata_en_frame(ptr_segmento, ptr_seg_metadata);
		}

		segmentheapmetadata *ptr_ultimo_seg_metadata = (segmentheapmetadata*)list_get(ptr_segmento->metadatas,ptr_segmento->metadatas->elements_count-1);
		heapmetadata *ptr_ultimo_metadata = (heapmetadata*)ptr_ultimo_seg_metadata->metadata;
		if(ptr_ultimo_seg_metadata->metadata->bytes>TAM_PAG){
			int cantidad_extra_de_memoria_en_pags = floor(ptr_ultimo_seg_metadata->metadata->bytes/TAM_PAG);

				for(int i=1;i<=cantidad_extra_de_memoria_en_pags;i++){
				void _liberar_pagina(void*element){
					page*ptr_pagina = (page*)element;
					bitarray_set_bit(BIT_ARRAY_FRAMES,ptr_pagina->nro_frame);
					free(ptr_pagina);
				}

				list_remove_and_destroy_element(ptr_segmento->tabla_de_paginas,ptr_segmento->tabla_de_paginas->elements_count-i,_liberar_pagina);
			}

			ptr_ultimo_metadata->bytes-=(cantidad_extra_de_memoria_en_pags*TAM_PAG);
			escribir_metadata_en_frame(ptr_segmento, ptr_ultimo_seg_metadata);

		}
		if(ptr_segmento->tabla_de_paginas->elements_count==1 && ptr_segmento->metadatas->elements_count==1){
			eliminar_segmento_de_tabla(proceso->tablaDeSegmentos,ptr_segmento);
		}
	}
	else
		raise(SIGABRT);
}

void* museget(t_proceso* proceso, t_list* paqueteRecibido){
	int cantidadDeBytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion = *((uint32_t*) list_get(paqueteRecibido, 2));
	void* buffer = malloc(cantidadDeBytes);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);

    if (!ptr_segmento || (direccion + cantidadDeBytes) > limite_segmento(ptr_segmento)) {
      free(buffer);
      raise(SIGSEGV); //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO O SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
    }

	int numeroPagina = numero_pagina(ptr_segmento, direccion);
	int desplazamientoEnPagina = desplazamiento_en_pagina(ptr_segmento,direccion);
	int auxiliar = 0;
	int tamanioACopiar;
	int desplazamientoEnBuffer = 0;
	while (cantidadDeBytes > 0) {
		page* pagina = (page*)list_get(ptr_segmento->tabla_de_paginas,numeroPagina + auxiliar);
		traer_pagina(pagina);
		frame* marco =(frame*) (pagina->frame);
		tamanioACopiar = minimo(cantidadDeBytes,(TAM_PAG - desplazamientoEnPagina));
		memcpy(buffer + desplazamientoEnBuffer,marco->memoria + desplazamientoEnPagina, tamanioACopiar);
		cantidadDeBytes -= tamanioACopiar;
		auxiliar++; //siguiente pagina
		desplazamientoEnBuffer += tamanioACopiar;
		desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
	  }

	return (buffer);
}

uint32_t musemap(t_proceso*proceso, char*path, size_t length, int flags){
	int paginas_de_seg;
	segment *segmento_mmap=crear_segmento(MMAP,length,proceso->tablaDeSegmentos);
	mapped_file *ptr_mapped_file_metadata;
	segmentmmapmetadata *ptr_metadata;
	int fd = open(path, O_RDWR | O_CREAT, S_IRUSR); // Lo abre para lectura si existe, sino lo crea
	struct stat statfile;
	if(fstat(fd,&statfile)==-1)
			return -1;

	if(flags == MAP_PRIVATE && !buscar_archivo_abierto(path)){
		void *file_mmap = mmap(NULL,length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
		segmento_mmap->tabla_de_paginas=crear_tabla_de_paginas(length);
		ptr_metadata=(segmentmmapmetadata*)malloc(sizeof(segmentmmapmetadata));
		ptr_metadata->path=string_duplicate(path);
		ptr_metadata->tam_mappeado=length;
		list_add(segmento_mmap->metadatas,ptr_metadata);
		ptr_mapped_file_metadata = (mapped_file*) malloc(sizeof(mapped_file));
		ptr_mapped_file_metadata->paginas_min_asignadas=list_create();
		ptr_mapped_file_metadata->flag=flags;
		ptr_mapped_file_metadata->file=file_mmap;
		ptr_mapped_file_metadata->path=string_duplicate(path);
		ptr_mapped_file_metadata->procesos=list_create();
		ptr_mapped_file_metadata->nro_file=MAPPED_FILES->elements_count;
		list_add(ptr_mapped_file_metadata->procesos,proceso);
		list_add(MAPPED_FILES,ptr_mapped_file_metadata);
		asignar_marcos_swap(segmento_mmap->tabla_de_paginas);
		escribir_en_archivo_swap(file_mmap,segmento_mmap->tabla_de_paginas,length);
	}else if (flags == MAP_SHARED && buscar_archivo_abierto(path)){
		ptr_mapped_file_metadata = buscar_archivo_abierto(path);
		ptr_metadata=(segmentmmapmetadata*)malloc(sizeof(segmentmmapmetadata));
		ptr_metadata->path=string_duplicate(path);
		ptr_metadata->tam_mappeado=statfile.st_size;
		list_add(segmento_mmap->metadatas,ptr_metadata);
		list_add(ptr_mapped_file_metadata->procesos,proceso);
		if(length>strlen((char*)ptr_mapped_file_metadata->file)){
			segmento_mmap->tabla_de_paginas=list_duplicate(ptr_mapped_file_metadata->paginas_min_asignadas);
			paginas_de_seg=ceil(((float)length)/TAM_PAG)-list_size(segmento_mmap->tabla_de_paginas);
			agregar_paginas_extras(segmento_mmap->tabla_de_paginas,paginas_de_seg);
		}
		if(length<strlen((char*)ptr_mapped_file_metadata->file)){
			paginas_de_seg=ceil(((float)length)/TAM_PAG);
			segmento_mmap->tabla_de_paginas=list_take(ptr_mapped_file_metadata->paginas_min_asignadas,paginas_de_seg);
		}
		if(length==strlen((char*)ptr_mapped_file_metadata->file))
			segmento_mmap->tabla_de_paginas=ptr_mapped_file_metadata->paginas_min_asignadas;
	}else if(flags == MAP_SHARED && !buscar_archivo_abierto(path)){
		void *file_mmap = mmap(NULL,statfile.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
		ptr_metadata=(segmentmmapmetadata*)malloc(sizeof(segmentmmapmetadata));
		ptr_metadata->tam_mappeado=statfile.st_size;
		ptr_metadata->path=string_duplicate(path);
		list_add(segmento_mmap->metadatas,ptr_metadata);
		ptr_mapped_file_metadata = (mapped_file*) malloc(sizeof(mapped_file));
		ptr_mapped_file_metadata->nro_file=MAPPED_FILES->elements_count;
		ptr_mapped_file_metadata->paginas_min_asignadas=crear_tabla_de_paginas(statfile.st_size);
		ptr_mapped_file_metadata->flag=flags;
		ptr_mapped_file_metadata->file=file_mmap;
		ptr_mapped_file_metadata->path=(char*)malloc(strlen(path)+1);
		strcpy(ptr_mapped_file_metadata->path,path);
		ptr_mapped_file_metadata->procesos=list_create();
		list_add(ptr_mapped_file_metadata->procesos,proceso);
		list_add(MAPPED_FILES,ptr_mapped_file_metadata);
		escribir_en_archivo_swap(file_mmap,ptr_mapped_file_metadata->paginas_min_asignadas,statfile.st_size);
		segmento_mmap->tamanio=length;
		paginas_de_seg=ceil(((float)length)/TAM_PAG);
		if(paginas_de_seg==list_size(ptr_mapped_file_metadata->paginas_min_asignadas))
			segmento_mmap->tabla_de_paginas=list_duplicate(ptr_mapped_file_metadata->paginas_min_asignadas);
		if(paginas_de_seg>list_size(ptr_mapped_file_metadata->paginas_min_asignadas)){
			segmento_mmap->tabla_de_paginas=list_duplicate(ptr_mapped_file_metadata->paginas_min_asignadas);
			agregar_paginas_extras(segmento_mmap->tabla_de_paginas,paginas_de_seg);
		}else{
			segmento_mmap->tabla_de_paginas=list_take(ptr_mapped_file_metadata->paginas_min_asignadas,paginas_de_seg);
		}
	}else
		return -1;
	close(fd);
    return segmento_mmap->base_logica;
}

mapped_file* buscar_archivo_abierto(char*path){
	bool _archivo_fue_abierto(void *element){
		mapped_file *ptr_mapped_file = (mapped_file*)element;
		return !strcmp(path,ptr_mapped_file->path);
	}
	return (mapped_file*)list_find(MAPPED_FILES,_archivo_fue_abierto);
}

int musesync(t_proceso* proceso,uint32_t direccion, size_t length){
	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);
	segmentmmapmetadata *ptr_metadata = (segmentmmapmetadata*) list_get(ptr_segmento->metadatas,0);
	mapped_file *ptr_mapped_file=buscar_archivo_abierto(ptr_metadata->path);
	void *file;
	if(direccion + length > limite_segmento(ptr_segmento))
	    raise(SIGSEGV);
	if(!ptr_segmento || ptr_segmento->tipo!=MMAP)
		raise(SIGABRT);

	if(ptr_mapped_file){
		int nro_pag = div(direccion-ptr_segmento->base_logica,TAM_PAG).quot;
		int offset_pag = div(direccion-ptr_segmento->base_logica,TAM_PAG).rem;
		size_t bytes_sincronizados = 0;
		int bytes_a_copiar=0;

		file = ptr_mapped_file->file;
		for(int i=nro_pag;bytes_sincronizados<length;i++){
			page *ptr_pagina = (page*)list_get(ptr_segmento->tabla_de_paginas,i);
			traer_pagina(ptr_pagina);
			frame *ptr_frame = (frame*) ptr_pagina->frame;
			if(i==nro_pag)
				bytes_a_copiar=TAM_PAG-offset_pag;
			else
				bytes_a_copiar=minimo(TAM_PAG,length-bytes_sincronizados);
			memcpy(file+direccion-ptr_segmento->base_logica+bytes_sincronizados,ptr_frame->memoria,bytes_a_copiar);
			bytes_sincronizados+=bytes_a_copiar;
			offset_pag=0; // Si la primera vez esta parado en el medio o casi al final y se puede, se posiciona ahi, despues ya no
		}
		return 0;
	}else
		return -1;
}

int museunmap(t_proceso *proceso,uint32_t direccion){
	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);
	segmentmmapmetadata *ptr_metadata = (segmentmmapmetadata*)list_get(ptr_segmento->metadatas,0);
	mapped_file *ptr_mapped_file = buscar_archivo_abierto(ptr_metadata->path);
	if(ptr_segmento->tipo==MMAP && ptr_mapped_file->flag==MAP_PRIVATE){
		list_remove(proceso->tablaDeSegmentos,ptr_segmento->nro_segmento);
		munmap(ptr_mapped_file->file,ptr_metadata->tam_mappeado);
		free(ptr_mapped_file->path);
//		list_destroy(ptr_mapped_file->procesos);
//		list_destroy(ptr_mapped_file->paginas_min_asignadas);
		recalcular_bases_logicas_de_segmentos(proceso->tablaDeSegmentos);
		liberar_recursos_del_segmento(ptr_segmento);
	}
//
	return 0; // Aca iria un quilombo de cosas, que voy a hacer una vez que tenga bien hecho el swap and stuff
}

int musecpy(t_proceso* proceso, t_list* paqueteRecibido) {

	int cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion_pedida = *((uint32_t*) list_get(paqueteRecibido, 3));

	void* buffer_a_copiar = malloc(cantidad_de_bytes);

	memcpy(buffer_a_copiar, list_get(paqueteRecibido, 2), cantidad_de_bytes);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion_pedida,
			proceso->tablaDeSegmentos);

	if (!ptr_segmento || (direccion_pedida + cantidad_de_bytes)> limite_segmento(ptr_segmento)) {
		free(buffer_a_copiar);
		raise(SIGSEGV);//ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
		//O SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
	}

	if (direccion_pisa_alguna_metadata(ptr_segmento, direccion_pedida,cantidad_de_bytes)) {
		free(buffer_a_copiar);
		return -1;
	}

	int numeroPagina = numero_pagina(ptr_segmento, direccion_pedida);
	int desplazamientoEnPagina = desplazamiento_en_pagina(ptr_segmento,direccion_pedida);

	int auxiliar = 0;
	int tamanioACopiar;
	int desplazamientoEnBuffer = 0;

	while (cantidad_de_bytes > 0) {
		page* pagina = (page*)list_get(ptr_segmento->tabla_de_paginas,numeroPagina + auxiliar);
		traer_pagina(pagina);
		frame* marco = pagina->frame;
		tamanioACopiar = minimo(cantidad_de_bytes, (TAM_PAG - desplazamientoEnPagina));

		memcpy((marco->memoria + desplazamientoEnPagina), (buffer_a_copiar + desplazamientoEnBuffer), tamanioACopiar);

		pagina->bit_modificado = true;

		cantidad_de_bytes -= tamanioACopiar;
		auxiliar++; //siguiente pagina
		desplazamientoEnBuffer += tamanioACopiar;
		desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
	}
	return 0;
}

segment* ultimo_segmento_heap(t_proceso* proceso){

	bool _segmentos_heap(void* elemento){
		segment* segmento = (segment*)elemento;
		return segmento->tipo == HEAP;
	}

	t_list* list_de_heap = list_filter(proceso->tablaDeSegmentos, &_segmentos_heap);
	segment* ultimoHeap = list_get(list_de_heap, list_size(list_de_heap)-1);
	return ultimoHeap;
}

int memoria_libre_en_segmento(segment* segmento){
	int seed = 0;
	bool _metadata_free(void* elemento){
		heapmetadata* metadata = (heapmetadata*)elemento;
		return !metadata->ocupado;
	}
	t_list* metadatasFree = list_filter(segmento->metadatas, &_metadata_free);

	void* _suma_bytes(void* seed, void*elemento){
		heapmetadata* metadata = (heapmetadata*)elemento;
		int* seedSum = (int*)seed;
		int suma = metadata->bytes + *(seedSum);
		memcpy(seed, &suma, sizeof(int));
		return seed;
	}
	return *((int*)list_fold(metadatasFree, &seed, &_suma_bytes));
}

int cantidad_total_de_segmentos_en_sistema(){
	int seed = 0;

	void* suma_segmentos(void* seed, void* element){
		t_proceso* proceso = (t_proceso*)element;
		int* seedSum = (int*)seed;
		int segments = list_size(proceso->tablaDeSegmentos);
		int suma = segments + *(seedSum);
		memcpy(seed, &suma, sizeof(int));
		return seed;

	}

	return *((int*)list_fold(PROCESS_TABLE, &seed, &suma_segmentos));
}

int suma_frames_libres(){
	int max = bitarray_get_max_bit(BIT_ARRAY_FRAMES);
	int contador = 0;
	for(int i=0; i<max;i++){
		if(!bitarray_test_bit(BIT_ARRAY_FRAMES, i))
			contador++;
	}
	return contador;
}

int memory_leaks_proceso(t_proceso* proceso){
	return proceso->totalMemoriaPedida - proceso->totalMemoriaLiberada;
}
