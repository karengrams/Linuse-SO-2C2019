#include "muse-server.h"

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

void inicializar_recursos_de_memoria(){
	config = leer_config();
	TAM_PAG = leer_del_config("PAGE_SIZE", config);
	inicilizar_tabla_de_frames();
	memoria = malloc(leer_del_config("MEMORY_SIZE", config));
	CANTIDAD_DE_MEMORIA_DISPONIBLE= leer_del_config("MEMORY_SIZE",config);
	memset(memoria,'\0',CANTIDAD_DE_MEMORIA_DISPONIBLE);
	dividir_memoria_en_frames(memoria, TAM_PAG, leer_del_config("MEMORY_SIZE", config));
	inicializar_bitmap();
	inicializar_tabla_procesos();
	inicializar_tabla_archivos_compartidos();
	inicializar_bitmap_swap(leer_del_config("SWAP_SIZE",config),TAM_PAG);
	inicializar_memoria_virtual(leer_del_config("SWAP_SIZE",config));
	sem_init(&mutex_frames,0,1);
	sem_init(&binary_swap_pages,0,0);
	sem_init(&mutex_swap,0,1);
	sem_init(&mutex_swap_file,0,1);
	sem_init(&mutex_shared_files,0,1);
	sem_init(&mutex_write_shared_files,0,1);
	sem_init(&mutex_clock_mod,0,1);
	sem_init(&mutex_write_frame,0,1);
	sem_init(&mutex_process_list,0,1);
	sem_init(&binary_free_frame,0,0);
	sem_init(&mutex_free,0,1);
	sem_init(&mutex_log,0,1);
	logger_info = log_create("MUSE.log","MUSE",true,LOG_LEVEL_INFO);
	logger_error = log_create("MUSE.log","MUSE",true,LOG_LEVEL_ERROR);
	logger_trace = log_create("MUSE.log","MUSE",true,LOG_LEVEL_TRACE);
}

void liberacion_de_recursos(int num){

	log_trace(logger_trace,"se procede a liberar recursos del sistema.");

	void _liberar_proceso(void*element){
		t_proceso *ptr_proceso = (t_proceso*) element;
		liberar_proceso(ptr_proceso);
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
	sem_destroy(&mutex_frames);
	sem_destroy(&mutex_log);
	sem_destroy(&mutex_swap);
	sem_destroy(&mutex_clock_mod);
	sem_destroy(&mutex_swap_file);
	sem_destroy(&mutex_shared_files);
	sem_destroy(&binary_swap_pages);
	sem_destroy(&mutex_write_frame);
	sem_destroy(&mutex_process_list);
	sem_destroy(&binary_free_frame);
	sem_destroy(&mutex_free);
	log_trace(logger_trace,"se liberaron correctamente todos los recursos.");
	log_destroy(logger_error);
	log_destroy(logger_info);
	log_destroy(logger_trace);
	raise(SIGTERM);
}

void loggear_informacion(t_proceso *proceso){
	sem_wait(&mutex_log);
	int cantidad_seg_tot = cantidad_total_de_segmentos_en_sistema();
	int porcentaje_de_memoria,espacio_libre_ultimo_metadata;
	if(!cantidad_seg_tot)
		porcentaje_de_memoria=0;
	else
		porcentaje_de_memoria = list_size(proceso->tablaDeSegmentos)/cantidad_seg_tot;
	segment* ptr_ultimo_seg = ultimo_segmento_heap(proceso);
	if(!ptr_ultimo_seg)
		espacio_libre_ultimo_metadata = 0;
	else
		espacio_libre_ultimo_metadata = espacio_libre(ptr_ultimo_seg);

	log_info(logger_info,"el porcentaje de memoria asignada del proceso #%d es igual a %d % . Se solicito un total de %d bytes y se libero un total de %d bytes. Se obtuvo un total de %d bytes perdidos. Y la cantidad de memoria disponible del proceso es de %d"
				,proceso->id
				,porcentaje_de_memoria
				,proceso->totalMemoriaPedida
				,proceso->totalMemoriaLiberada
				,memory_leaks_proceso(proceso)
				,espacio_libre_ultimo_metadata);
	sem_post(&mutex_log);
}

int museinit(t_proceso* cliente_a_atender, char* ipCliente, int id){
	log_trace(logger_trace,"proceso #%d solicito la apertura de la libreria.",id);
	if (cliente_a_atender != NULL) {
		return -1; //YA EXISTE EN NUESTRA TABLA ERROR
	} else {
		t_proceso* procesoNuevo = crear_proceso(id, ipCliente);
		sem_wait(&mutex_process_list);
		list_add(PROCESS_TABLE, procesoNuevo); //Si no existe lo creamos y agregamos
		sem_post(&mutex_process_list);
		log_trace(logger_trace,"se inicializo correctamente la libreria para proceso #%d",procesoNuevo->id);
		return 0;
	}
}

void liberar_proceso(t_proceso*proceso){
	liberar_tabla_de_segmentos(proceso);
		bool _mismo_id (void*element){
			t_proceso *otroproceso = (t_proceso*)element;
			return (otroproceso->id == proceso->id);
		}

		list_remove_by_condition(PROCESS_TABLE,_mismo_id);
		free(proceso->ip);
		free(proceso);
}

void museclose(t_proceso* proceso){
	log_trace(logger_trace,"proceso #%d solicito el cierre de la libreria.",proceso->id);
	loggear_informacion(proceso);
	liberar_proceso(proceso);

}



uint32_t musealloc(t_proceso* proceso,int tam){
	log_trace(logger_trace,"el proceso #%d solicito %d bytes dinamicos de memoria.",proceso->id,tam);
	loggear_informacion(proceso);
	uint32_t offset;
	t_list *tabla_de_segmento = proceso->tablaDeSegmentos;
	segment *ptr_segmento = buscar_segmento_heap_para_tam(tabla_de_segmento,tam); // Busca segmento de tipo heap con espacio
	if(!ptr_segmento){
		ptr_segmento= buscar_segmento_heap_expandible_para_tam(tabla_de_segmento,tam);
		if(ptr_segmento){
			log_trace(logger_trace,"se procede a extender el segmento nro #%d del proceso #%d.",ptr_segmento->nro_segmento,proceso->id);
			expandir_segmento(ptr_segmento,tam);
		}
		else{
			log_trace(logger_trace,"se crea un segmento nuevo para el proceso #%d.",proceso->id);
			ptr_segmento=crear_segmento(HEAP,tam+sizeof(heapmetadata),tabla_de_segmento);
		}
	}else{
		log_trace(logger_trace,"se procede a asignar memoria del segmento nro #%d del proceso #%d.",ptr_segmento->nro_segmento,proceso->id);
	}
	offset = obtener_offset_para_tam(ptr_segmento,tam);

	segmentheapmetadata *paux_seg_metadata_ocupado=buscar_metadata_de_segmento_segun(offset,ptr_segmento);
	heapmetadata *paux_metadata_ocupado = paux_seg_metadata_ocupado->metadata;

	if(paux_metadata_ocupado->bytes-tam){ // Si los bytes que tenia libre antes de modificarlo, es mayor a lo pedido +
		// el tam. del heapmetadata, deberia ir el metadata de libre
		if(paux_metadata_ocupado->bytes-tam<sizeof(heapmetadata)){
			int cantidad_de_paginas= paginas_necesarias(sizeof(heapmetadata)-(paux_metadata_ocupado->bytes-tam));
			agregar_paginas(ptr_segmento->tabla_de_paginas,cantidad_de_paginas,ptr_segmento->tabla_de_paginas->elements_count);
			paux_metadata_ocupado->bytes+=cantidad_de_paginas*TAM_PAG;
		}
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
	log_trace(logger_trace,"se le asigno correctamente memoria dinamica al proceso #%d. La direccion logica es %lu",proceso->id,ptr_segmento->base_logica+offset+sizeof(heapmetadata));
	return ptr_segmento->base_logica+offset+sizeof(heapmetadata);
}

int musefree(t_proceso *proceso, uint32_t direccion) {
	sem_wait(&mutex_free);
	log_trace(logger_trace,"el proceso #%d solicito la liberacion de la direccion de memoria %lu.",proceso->id,direccion);

	segment *ptr_segmento = buscar_segmento_dada_una_direccion(direccion,proceso->tablaDeSegmentos);
	if(ptr_segmento && ptr_segmento->tipo==HEAP){
		segmentheapmetadata *ptr_seg_metadata = buscar_metadata_para_liberar(direccion - ptr_segmento->base_logica, ptr_segmento);
		if (ptr_seg_metadata) {
			proceso->totalMemoriaLiberada += ptr_seg_metadata->metadata->bytes; //sumo la cant de bytes liberados
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
					int ultima_pagina = list_size(ptr_segmento->tabla_de_paginas);
					page *ptr_pag = (page*)list_get(ptr_segmento->tabla_de_paginas,ultima_pagina-1);
					eliminar_pagina_de_segmento(ptr_segmento,ptr_pag);
				}

			ptr_ultimo_metadata->bytes-=(cantidad_extra_de_memoria_en_pags*TAM_PAG);

			escribir_metadata_en_frame(ptr_segmento, ptr_ultimo_seg_metadata);

		}
		if(ptr_segmento->tabla_de_paginas->elements_count==1 && ptr_segmento->metadatas->elements_count==1){
			eliminar_segmento_de_tabla(proceso,ptr_segmento);
		}
		log_trace(logger_trace,"se libero correctamente la direccion %lu del proceso #%d",direccion,proceso->id);
		sem_post(&mutex_free);
		return 0;
	}
	else{
		log_error(logger_error,"error al liberar la memoria de la direccion %lu.",direccion);
		sem_post(&mutex_free);
		return -1;
	}
}

void* museget(t_proceso* proceso, t_list* paqueteRecibido){
	sem_wait(&mutex_swap);
	int cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion = *((uint32_t*) list_get(paqueteRecibido, 2));
	void* buffer = malloc(cantidad_de_bytes);
	log_trace(logger_trace,"el proceso #%d solicito la lectura de %d bytes de la direccion %lu",proceso->id,cantidad_de_bytes,direccion);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);

	if (!ptr_segmento) {
		free(buffer);
		log_error(logger_error,"la direccion %lu no corresponde a un segmento del proceso #%d",direccion,proceso->id);
		sem_post(&mutex_swap);
		liberar_proceso(proceso);
		return NULL; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
	}

	if ((direccion + cantidad_de_bytes) > limite_segmento(ptr_segmento)+1) {
		free(buffer);
		log_error(logger_error,"la direccion %lu se pasa del limite del segmento #%d del proceso #%d",direccion,ptr_segmento->nro_segmento,proceso->id);
		return NULL; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
	}

	int page_number = numero_pagina(ptr_segmento, direccion);
	int page_offset = desplazamiento_en_pagina(ptr_segmento,direccion);
	int offset = 0;
	int copied_bytes=0;
	int copied_bytes_in_frame = 0;

	while(copied_bytes<cantidad_de_bytes){
		page *ptr_page = (page*)list_get(ptr_segmento->tabla_de_paginas,page_number+offset);
		traer_pagina(ptr_page);
		ptr_page->bit_uso=true;
		frame * ptr_frame = ptr_page->frame;
		memcpy(buffer+copied_bytes,ptr_frame->memoria+page_offset+copied_bytes_in_frame,1);
		copied_bytes++;
		copied_bytes_in_frame++;
		if(copied_bytes+page_offset==TAM_PAG && ptr_page->nro_pagina==page_number){
			offset ++;
			copied_bytes_in_frame=0;
			page_offset=0;
		} else if(copied_bytes_in_frame==TAM_PAG){
			offset++;
			copied_bytes_in_frame=0;
		}
	}
	sem_post(&mutex_swap);
	return (buffer);
}

uint32_t musemap(t_proceso*proceso, char*path, size_t length, int flags){
	if(flags==MAP_SHARED)
		log_trace(logger_trace,"el proceso #%d solicito el mappeo de %d bytes del archivo '%s' de forma compartida.",proceso->id,(int)length,path);
	else
		log_trace(logger_trace,"el proceso #%d solicito el mappeo de %d bytes del archivo '%s' de forma privada.",proceso->id,(int)length,path);

	int paginas_de_seg,tam_a_mappear;
	void *file_mmap;
	segment *segmento_mmap=crear_segmento(MMAP,length,proceso->tablaDeSegmentos);
	mapped_file *ptr_mapped_file_metadata = buscar_archivo_abierto(path);
	segmentmmapmetadata *ptr_metadata=(segmentmmapmetadata*)malloc(sizeof(segmentmmapmetadata));
	int fd = open(path, O_RDWR , S_IRUSR | S_IWUSR); // Lo abre para lectura si existe, sino lo crea

	struct stat statfile;
	if(fstat(fd,&statfile)==-1)
		return -1; // TODO: agregar signal

	if(flags==MAP_PRIVATE)
		tam_a_mappear=length;
	else
		tam_a_mappear=statfile.st_size;

	if(flags==MAP_PRIVATE || (flags == MAP_SHARED && !ptr_mapped_file_metadata)){
		file_mmap = mmap(NULL,tam_a_mappear,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
		ptr_metadata->path=string_duplicate(path);
		ptr_metadata->tam_mappeado=length;
		list_add(segmento_mmap->metadatas,ptr_metadata);
		sem_wait(&mutex_shared_files);
		ptr_mapped_file_metadata=(mapped_file*) malloc(sizeof(mapped_file));
		ptr_mapped_file_metadata->paginas_min_asignadas=crear_tabla_de_paginas(statfile.st_size);
		ptr_mapped_file_metadata->flag=flags;
		ptr_mapped_file_metadata->file=file_mmap;
		ptr_mapped_file_metadata->path=string_duplicate(path);
		ptr_mapped_file_metadata->procesos=list_create();
		ptr_mapped_file_metadata->nro_file=MAPPED_FILES->elements_count;
		list_add(MAPPED_FILES,ptr_mapped_file_metadata);
		list_add(ptr_mapped_file_metadata->procesos,proceso);
		sem_post(&mutex_shared_files);

		sem_wait(&mutex_swap);
		asignar_marcos_swap(ptr_mapped_file_metadata->paginas_min_asignadas);
		sem_post(&mutex_swap);
		escribir_en_archivo_swap(file_mmap,ptr_mapped_file_metadata->paginas_min_asignadas,tam_a_mappear,statfile.st_size);

		paginas_de_seg=ceil(((float)length)/TAM_PAG);

		if(paginas_de_seg==list_size(ptr_mapped_file_metadata->paginas_min_asignadas))
			segmento_mmap->tabla_de_paginas=list_duplicate(ptr_mapped_file_metadata->paginas_min_asignadas);
		else if(paginas_de_seg>list_size(ptr_mapped_file_metadata->paginas_min_asignadas)){
			segmento_mmap->tabla_de_paginas=list_duplicate(ptr_mapped_file_metadata->paginas_min_asignadas);
			agregar_paginas_extras(segmento_mmap->tabla_de_paginas,paginas_de_seg);
		}else{
			segmento_mmap->tabla_de_paginas=list_take(ptr_mapped_file_metadata->paginas_min_asignadas,paginas_de_seg);
		}

	}else if (flags == MAP_SHARED && ptr_mapped_file_metadata && ptr_mapped_file_metadata->flag==MAP_SHARED){
		ptr_metadata->path=string_duplicate(path);
		ptr_metadata->tam_mappeado=length;
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
	}

	log_trace(logger_trace,"se le otorgo la direccion %lu del archivo mappeado a memoria al proceso #%d.",segmento_mmap->base_logica,proceso->id);

	close(fd);
    return segmento_mmap->base_logica;
}

int musesync(t_proceso* proceso,uint32_t direccion, size_t length){
	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);
	segmentmmapmetadata *ptr_metadata = (segmentmmapmetadata*) list_get(ptr_segmento->metadatas,0);
	mapped_file *ptr_mapped_file=buscar_archivo_abierto(ptr_metadata->path);
	void *file;
	if(direccion + length > limite_segmento(ptr_segmento)+1){
		log_error(logger_error,"la cantidad de bytes a sincronizar del segmento #%d del proceso #%d se pasa del limite del segmento. Se produce segmentation fault.",ptr_segmento->nro_segmento,proceso->id);
	    loggear_informacion(proceso);
		return -2;
	}
	if(!ptr_segmento || ptr_segmento->tipo!=MMAP){
		log_error(logger_error,"la direccion #lu solicitada a sincronizar por proceso #%d es direccion invalida.",direccion,proceso->id);
		return -3;
	}

	if(ptr_mapped_file){
		log_trace(logger_trace,"el proceso #%d solicito la sincronizacion de %d bytes al archivo %s.",proceso->id,(int)length,ptr_mapped_file->path);
		int nro_pag = div(direccion-ptr_segmento->base_logica,TAM_PAG).quot;
		int cantidad_de_paginas = div(direccion-ptr_segmento->base_logica+length,TAM_PAG).quot-nro_pag;
		int paginas_copiadas=0;

		file = ptr_mapped_file->file;
		sem_wait(&mutex_write_shared_files);
		for(int i=nro_pag;paginas_copiadas<=cantidad_de_paginas;i++){
			page *ptr_pagina = (page*)list_get(ptr_segmento->tabla_de_paginas,i);
			traer_pagina(ptr_pagina);
			frame *ptr_frame = (frame*) ptr_pagina->frame;
			memcpy(file+paginas_copiadas*TAM_PAG,ptr_frame->memoria,TAM_PAG);
			paginas_copiadas++;
		}
		sem_post(&mutex_write_shared_files);

		return 0;
	}else
		return -1;
}

int museunmap(t_proceso *proceso,uint32_t direccion){
	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);
	segmentmmapmetadata *ptr_metadata = (segmentmmapmetadata*)list_get(ptr_segmento->metadatas,0);
	mapped_file *ptr_mapped_metadata = buscar_archivo_abierto(ptr_metadata->path);

	if(ptr_mapped_metadata){
		log_trace(logger_trace,"el proceso #%d solicito la liberacion del archivo '%s'.",proceso->id,ptr_mapped_metadata->path);
		liberar_recursos_segmento_map(ptr_segmento,proceso);
		return 0;
	}
	else{
		log_error(logger_error,"la direccion solicitada, por el proceso#%d, a liberar no corresponde a un archivo mappeado a memoria.",proceso->id);
		return -1;
	}
}

int musecpy(t_proceso* proceso, t_list* paqueteRecibido) {
	sem_wait(&mutex_frames);

	int cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion_pedida = *((uint32_t*) list_get(paqueteRecibido, 3));
	log_trace(logger_trace,"el proceso #%d solicito copiar %d a copiar en la direccion #%lu.",proceso->id,cantidad_de_bytes,direccion_pedida);

	void* buffer_a_copiar = malloc(cantidad_de_bytes);

	memcpy(buffer_a_copiar, list_get(paqueteRecibido, 2), cantidad_de_bytes);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion_pedida,proceso->tablaDeSegmentos);

	if (!ptr_segmento || (direccion_pedida + cantidad_de_bytes)> limite_segmento(ptr_segmento)+1) {
		free(buffer_a_copiar);
		log_error(logger_error,"la direccion pedida por proceso #%d no corresponde a un segmento del mismo.",proceso->id);
		return -2;//ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
		//O SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
	}

	if(ptr_segmento->tipo==HEAP)
		if (direccion_pisa_alguna_metadata(ptr_segmento, direccion_pedida,cantidad_de_bytes)) {
			log_error(logger_error,"la cantidad de %d bytes por proceso #%d excede el limite.",cantidad_de_bytes,proceso->id);
			free(buffer_a_copiar);
			return -1;
		}

	int page_number = numero_pagina(ptr_segmento, direccion_pedida);
	int page_offset = desplazamiento_en_pagina(ptr_segmento,direccion_pedida);
	int copied_bytes = 0;
	int offset=0;
	int copied_bytes_frame=0;

	while(copied_bytes<cantidad_de_bytes){
		page *pagina = (page*)list_get(ptr_segmento->tabla_de_paginas,page_number+offset);
		traer_pagina(pagina);
		frame*marco=pagina->frame;
		pagina->bit_modificado=true;
		pagina->bit_uso=true;
		memcpy(marco->memoria+page_offset+copied_bytes_frame,buffer_a_copiar+copied_bytes,1);
		copied_bytes++;
		copied_bytes_frame++;
		if(copied_bytes+page_offset==TAM_PAG && pagina->nro_pagina==page_number){
			offset++;
			page_offset=0;
			copied_bytes_frame=0;
		} else if(copied_bytes_frame==TAM_PAG){
			offset++;
			copied_bytes_frame=0;
		}
	}
	sem_post(&mutex_frames);

	return 0;
}

segment* ultimo_segmento_heap(t_proceso* proceso){

	segment*ptr_seg = NULL;

	void _buscar_ultimo_segmento_tipo_heap(void*element){
		segment *otro_ptr_seg = (segment*)element;
		if(otro_ptr_seg->tipo==HEAP){
			ptr_seg=otro_ptr_seg;
		}
	}

	list_iterate(proceso->tablaDeSegmentos,_buscar_ultimo_segmento_tipo_heap);

	return ptr_seg;

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
	int cantidad_total_de_segmentos=0;
	sem_wait(&mutex_process_list);
	void _cantidad_de_segmentos(void*element){
		t_proceso *proceso = (t_proceso*)element;
		cantidad_total_de_segmentos+=list_size(proceso->tablaDeSegmentos);
	}

	list_iterate(PROCESS_TABLE,_cantidad_de_segmentos);
	sem_post(&mutex_process_list);

	return cantidad_total_de_segmentos;
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

t_proceso* buscar_proceso(t_list* paqueteRecibido, char* ipCliente){
	int id = *((int*) list_get(paqueteRecibido, 0)); // ACA se muere?
	t_proceso *process;

	bool mismoipid(void* arg) {
		t_proceso* cliente = (t_proceso*) arg;
		return ((cliente->id) == id && !(strcmp(ipCliente, cliente->ip)));
	}

	sem_wait(&mutex_process_list);
	process = list_find(PROCESS_TABLE, mismoipid);
	sem_post(&mutex_process_list);
	return process;

}
