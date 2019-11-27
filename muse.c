#include "muse.h"

#define ERROR -1;

void inicializar_tabla_procesos(){
	PROCESS_TABLE = list_create();
}

void inicializar_tabla_archivos_compartidos(){
	MAPPED_SHARED_FILES = list_create();
}

t_config* leer_config(){
	return config_create("muse.config");
}

int leer_del_config(char* valor, t_config* archivo_config) {
	return config_get_int_value(archivo_config, valor);
}

void liberacion_de_recursos(void*mem, t_config *config){
	config_destroy(config);
	free(mem);
	void _destroy_element(void *elemento) {
		frame *marco = (frame*) elemento;
		free(marco);
	}
	list_destroy_and_destroy_elements(FRAMES_TABLE, &_destroy_element);
	free(BIT_ARRAY_FRAMES->bitarray);
	bitarray_destroy(BIT_ARRAY_FRAMES);
}

int muse_init(t_proceso* cliente_a_atender, char* ipCliente, int id){
	if (cliente_a_atender != NULL) {
		return ERROR; //YA EXISTE EN NUESTRA TABLA ERROR

	} else {
		t_proceso* procesoNuevo = crear_proceso(id, ipCliente);
		list_add(PROCESS_TABLE, procesoNuevo); //Si no existe lo creamos y agregamos
		return 0;
	}
}

uint32_t muse_alloc(t_proceso* proceso,int tam){
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

	segmentmetadata *paux_seg_metadata_ocupado = buscar_metadata_de_segmento_segun(offset,ptr_segmento);
	heapmetadata *paux_metadata_ocupado = paux_seg_metadata_ocupado->metadata;

	if(paux_metadata_ocupado->bytes>tam+sizeof(heapmetadata)){ // Si los bytes que tenia libre antes de modificarlo, es mayor a lo pedido +
		// el tam. del heapmetadata, deberia ir el metadata de libre
		segmentmetadata *paux_seg_metadata_libre= (segmentmetadata*)malloc(sizeof(segmentmetadata));
		heapmetadata *paux_metadata_libre = (heapmetadata*) malloc(sizeof(heapmetadata));
		paux_metadata_libre->bytes=paux_metadata_ocupado->bytes-tam-sizeof(heapmetadata);
		paux_metadata_libre->ocupado=false;
		paux_seg_metadata_libre->posicion_inicial=offset+sizeof(heapmetadata)+tam;
		paux_seg_metadata_libre->metadata=paux_metadata_libre;
		list_add(ptr_segmento->metadatas,paux_seg_metadata_libre);
	}

	paux_metadata_ocupado->bytes=tam;
	paux_metadata_ocupado->ocupado=true;
	ptr_segmento->tamanio+=tam;
	return ptr_segmento->base_logica+offset+sizeof(heapmetadata);
}

void muse_free(t_proceso *proceso, uint32_t direccion) {
	segment *ptr_segmento = buscar_segmento_dada_una_direccion(direccion,proceso->tablaDeSegmentos);
	if(ptr_segmento->tipo==HEAP){
		segmentmetadata *ptr_seg_metadata = buscar_metadata_para_liberar(direccion - ptr_segmento->base_logica, ptr_segmento);
		if (ptr_seg_metadata) {
			heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
			ptr_metadata->ocupado = false;
			buddy_system(ptr_seg_metadata, ptr_segmento->metadatas);
			escribir_metadata_en_frame(ptr_segmento, ptr_seg_metadata);
		}
	}

}

void* muse_get(t_proceso* proceso, t_list* paqueteRecibido){

	int cantidadDeBytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion = *((uint32_t*) list_get(paqueteRecibido, 2));
	void* buffer = malloc(cantidadDeBytes);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);

    if (!ptr_segmento && (direccion + cantidadDeBytes) > limite_segmento(ptr_segmento)) {
      free(buffer);
      return NULL; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO O SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
    }

	int numeroPagina = numero_pagina(ptr_segmento, direccion);
	int desplazamientoEnPagina = desplazamiento_en_pagina(ptr_segmento,direccion);
	int auxiliar = 0;
	int tamanioACopiar;
	int desplazamientoEnBuffer = 0;
	while (cantidadDeBytes > 0) {
	  page* pagina = (page*)list_get(ptr_segmento->tabla_de_paginas,numeroPagina + auxiliar);
	  if(pagina->bit_presencia){
		  frame* marco =(frame*) (pagina->frame);
		  tamanioACopiar = minimo(cantidadDeBytes,(TAM_PAG - desplazamientoEnPagina));
		  memcpy(buffer + desplazamientoEnBuffer,marco->memoria + desplazamientoEnPagina, tamanioACopiar);
		  cantidadDeBytes -= tamanioACopiar;
		  auxiliar++; //siguiente pagina
		  desplazamientoEnBuffer += tamanioACopiar;
		  desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
	  }
	  else{//Page fault
		  printf("page fault");
		  cantidadDeBytes=0;
	  }
	}

	return (buffer);
}

/*
 * Crea el segmento mmap. Luego abre el fd para mappear el archivo en memoria. Cuando se produzca un
 * page fault. Se deberia buscar sus paginas, es decir, se debera mappear el
 * archivo y de ahi, deberia ir buscando la direccion que se le pide.
 */
mmapmetadata* crear_mmapmetadata(char *path, size_t length, int flags,segment* segmento_mmap){
	mmapmetadata *ptr_metadata = (mmapmetadata*) malloc(sizeof(mmapmetadata));
	(*ptr_metadata).path= (char*) malloc(strlen(path));
	strcpy(ptr_metadata->path,path);
	(*ptr_metadata).posicion_inicial=(*segmento_mmap).base_logica;
	(*ptr_metadata).bytes=length;
	(*ptr_metadata).flags=flags;
	list_add((*segmento_mmap).metadatas,ptr_metadata);
	return ptr_metadata;
}

uint32_t muse_map(t_proceso*proceso, char*path, size_t length, int flags){
	segment *segmento_mmap = crear_segmento(MMAP,length,proceso->tablaDeSegmentos);
	mmapmetadata *ptr_metadata = crear_mmapmetadata(path,length,flags,segmento_mmap);
	if(flags == MAP_SHARED && buscar_archivo_abierto(path)){ // Si es compartido y ya se abrio antes se debe buscar el file_mmap abiero
		mapped_shared_file *ptr_open_file=buscar_archivo_abierto(path);
		(*ptr_metadata).ptr_file=(*ptr_open_file).file_mmap;
	}else{ // No es compartido, o no se abrio antes
		int fd = open(path, O_RDWR | O_CREAT, S_IRUSR); // Lo abre para lectura si existe, sino lo crea
		struct stat statfile;
		if(fstat(fd,&statfile)==-1)
			perror("ERROR: couldn't get file size");
		void *file_mmap = mmap(NULL,length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
		ptr_metadata->ptr_file=file_mmap;
		close(fd);
		if(flags == MAP_SHARED){ // Es un archivo compartido que se abrio por primera vez
			mapped_shared_file *ptr_shared_file = (mapped_shared_file*)malloc(sizeof(mapped_shared_file));
			(*ptr_shared_file).path=(char*)malloc(strlen(path));
			strcpy(ptr_shared_file->path,path);
			ptr_shared_file->procesos=list_create();
			list_add(ptr_shared_file->procesos,proceso);
			(*ptr_shared_file).file_mmap=file_mmap;
			list_add(MAPPED_SHARED_FILES,ptr_shared_file);
		}
	}
    return segmento_mmap->base_logica;
}

mapped_shared_file* buscar_archivo_abierto(char*path){
	bool _archivo_fue_abierto(void *element){
		mapped_shared_file *ptr_mapped_file = (mapped_shared_file*)element;
		return !strcmp(path,ptr_mapped_file->path);
	}
	return (mapped_shared_file*)list_find(MAPPED_SHARED_FILES,_archivo_fue_abierto);
}

int muse_sync(t_proceso* proceso,uint32_t direccion, size_t length){
	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion, proceso->tablaDeSegmentos);

	// Esto era para ver si funcionaba bien
	harcodeo_un_poquito_las_cosas(ptr_segmento);
	hardcodeo_para_muse_sync(ptr_segmento);

	if(ptr_segmento->tipo==MMAP){
		int nro_pag = div(direccion-ptr_segmento->base_logica,TAM_PAG).quot;
		int offset_pag = div(direccion-ptr_segmento->base_logica,TAM_PAG).rem;
		size_t bytes_sincronizados = 0;

		if(!ptr_segmento && (direccion + length) > limite_segmento(ptr_segmento))
			return -1; // No encontro el segmento o se fue de limite

		mmapmetadata *ptr_metadata = list_get(ptr_segmento->metadatas,0);

		void _sincronizar_con_archivo_en_disco(void *element){
			page *ptr_pagina = (page*)element;
			if(posicion_en_tabla_paginas(ptr_pagina,ptr_segmento->tabla_de_paginas)>=nro_pag){
				while(!ptr_pagina->bit_presencia)
					perror("page fault\n"); // Aca hay que fijarse si es compartido, en caso de que lo sea, deberia buscar los frames
				frame *ptr_frame = ptr_pagina->frame;
				memcpy(ptr_metadata->ptr_file+direccion-ptr_segmento->base_logica+bytes_sincronizados,ptr_frame->memoria,TAM_PAG);
				bytes_sincronizados+=TAM_PAG;
			}
		}

		list_iterate(ptr_segmento->tabla_de_paginas,_sincronizar_con_archivo_en_disco);
		return 0;
	}
	else // Puede que no sea un segmento heap
		return -1;
}

int muse_unmap(uint32_t dir){
	return 0; // Aca iria un quilombo de cosas, que voy a hacer una vez que tenga bien hecho el swap and stuff
}

int muse_cpy(t_proceso* proceso, t_list* paqueteRecibido) {

	int cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion_pedida = *((uint32_t*) list_get(paqueteRecibido, 3));

	void* buffer_a_copiar = malloc(cantidad_de_bytes);

	memcpy(buffer_a_copiar, list_get(paqueteRecibido, 2), cantidad_de_bytes);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(direccion_pedida,
			proceso->tablaDeSegmentos);

	if (!ptr_segmento && (direccion_pedida + cantidad_de_bytes)> limite_segmento(ptr_segmento)) {
		free(buffer_a_copiar);
		return -1; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
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
		frame* marco = pagina->frame;
		tamanioACopiar = minimo(cantidad_de_bytes, (TAM_PAG - desplazamientoEnPagina));

		memcpy((marco->memoria + desplazamientoEnPagina), (buffer_a_copiar + desplazamientoEnBuffer), tamanioACopiar);

		cantidad_de_bytes -= tamanioACopiar;
		auxiliar++; //siguiente pagina
		desplazamientoEnBuffer += tamanioACopiar;
		desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
	}
	return 0;
}

void hardcodeo_para_muse_sync(segment* segmento_map){
	void _modificar_marcos(void *element){
		page* ptr_pagina = (page*)element;
		frame* ptr_marco = ptr_pagina->frame;
		memcpy(ptr_marco->memoria,"Hola, estamos probando a ver si funciona muse_sync",TAM_PAG);
	}

	list_iterate(segmento_map->tabla_de_paginas,_modificar_marcos);
}

void harcodeo_un_poquito_las_cosas(segment* segmento_map){
	char *buffer = malloc(TAM_PAG);
	int offset = 0;
	mmapmetadata *ptr_metadata = list_get(segmento_map->metadatas,0);
	int tam_archivo =ptr_metadata->bytes;
	cambiar_bit_de_prescencia(segmento_map->tabla_de_paginas);
	asignar_marcos(segmento_map->tabla_de_paginas);
	void *memoria_a_copiar = ptr_metadata->ptr_file;

	void _copiar_en_frame_info(void*element){
		page *ptr_page = (page*)element;
		frame *ptr_frame = ptr_page->frame;
		memcpy(ptr_frame->memoria,memoria_a_copiar+offset,TAM_PAG);
		offset+=TAM_PAG;
	}

	list_iterate(segmento_map->tabla_de_paginas,_copiar_en_frame_info);

}

int main(void) {
	inicilizar_tabla_de_frames();
	inicializar_bitmap();
	inicializar_tabla_procesos();
	inicializar_tabla_archivos_compartidos();
	t_config* config = leer_config();
	void *memoria = malloc(leer_del_config("MEMORY_SIZE", config));
	dividir_memoria_en_frames(memoria, leer_del_config("PAGE_SIZE", config), leer_del_config("MEMORY_SIZE", config));
	TAM_PAG = leer_del_config("PAGE_SIZE", config);
//	Arranca a atender clientes
	fd_set master;
	fd_set read_fds;
	int fdmax;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	int socketEs = iniciar_socket_escucha("127.0.0.1",config_get_string_value(config, "LISTEN_PORT"));

	FD_SET(socketEs, &master);
	fdmax = socketEs;

	while (1) {

		read_fds = master;
		select(fdmax + 1, &read_fds, NULL, NULL, NULL); // @suppress("Symbol is not resolved")

		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == socketEs) {
					admitir_nuevo_cliente(&master, &fdmax, i);
				} else {
					atender_cliente(&master, i);
				}
			}
		}
	}

	liberacion_de_recursos(memoria, config);

	return 0;
}

