#include "muse.h"

t_config* leer_config() {
	return config_create("/home/utnso/Escritorio/muse.config");
}

int leer_del_config(char* valor, t_config* archivo_config) {
	return config_get_int_value(archivo_config, valor);
}

void liberacion_de_recursos(void*mem, t_config *config) {
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

int muse_init(t_proceso* cliente_a_atender, char* ipCliente, int id) {
	if (cliente_a_atender != NULL) {
		return ERROR; //YA EXISTE EN NUESTRA TABLA ERROR

	} else {
		t_proceso* procesoNuevo = crear_proceso(id, ipCliente);
		list_add(PROCESS_TABLE, procesoNuevo); //Si no existe lo creamos y agregamos
		return 0;
	}
}

uint32_t muse_alloc(t_proceso* proceso, int tam) {
	uint32_t offset;
	t_list *tabla_de_segmento = proceso->tablaDeSegmentos;
	segment *ptr_segmento = buscar_segmento_heap_para_tam(tabla_de_segmento,tam); // Busca segmento de tipo heap con espacio
	if (!ptr_segmento) {
		ptr_segmento = buscar_segmento_heap_expandible_para_tam(tabla_de_segmento, tam);

		if (ptr_segmento)
			expandir_segmento(ptr_segmento, tam);

		else
			ptr_segmento = crear_segmento(HEAP, tam, tabla_de_segmento); //Pongo 0 en tamanio porque despues se lo sumas
	}

	offset = obtener_offset_para_tam(ptr_segmento, tam);

	segmentmetadata *paux_seg_metadata_ocupado = buscar_metadata_de_segmento_segun(offset, ptr_segmento);
	heapmetadata *paux_metadata_ocupado = paux_seg_metadata_ocupado->metadata;

	if (paux_metadata_ocupado->bytes != tam) {
		segmentmetadata *paux_seg_metadata_libre = (segmentmetadata*) malloc(sizeof(segmentmetadata));
		heapmetadata *paux_metadata_libre = (heapmetadata*) malloc(sizeof(heapmetadata));

		paux_metadata_libre->bytes = paux_metadata_ocupado->bytes - tam - sizeof(heapmetadata);
		paux_metadata_libre->ocupado = false;

		paux_seg_metadata_libre->posicion_inicial = offset + sizeof(heapmetadata) + tam;
		paux_seg_metadata_libre->metadata = paux_metadata_libre;
		list_add(ptr_segmento->metadatas, paux_seg_metadata_libre);
		escribir_metadata_en_frame(ptr_segmento, paux_seg_metadata_libre);
	}

	paux_metadata_ocupado->bytes = tam;
	paux_metadata_ocupado->ocupado = true;
	escribir_metadata_en_frame(ptr_segmento, paux_seg_metadata_ocupado);
	ptr_segmento->tamanio += tam;
	return ptr_segmento->base_logica + offset + sizeof(heapmetadata);
}

void muse_free(t_proceso *proceso, uint32_t direccion) {
	segment *ptr_segmento = buscar_segmento_dada_una_direccion(direccion,
			proceso->tablaDeSegmentos);
	segmentmetadata *ptr_seg_metadata = buscar_metadata_para_liberar(
			direccion - ptr_segmento->base_logica, ptr_segmento);
	if (ptr_seg_metadata) {
		heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
		ptr_metadata->ocupado = false;
		buddy_system(ptr_seg_metadata, ptr_segmento->metadatas);
		escribir_metadata_en_frame(ptr_segmento, ptr_seg_metadata);
	}

}

void* muse_get(t_proceso* proceso, t_list* paqueteRecibido) {
	int cantidadDeBytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion = *((uint32_t*) list_get(paqueteRecibido, 2));

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(
			proceso->tablaDeSegmentos, direccion);
	void* buffer = malloc(cantidadDeBytes);

	if (ptr_segmento == NULL) {
		free(buffer);
		return NULL; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
	}

	if ((direccion + cantidadDeBytes) > limite_segmento(ptr_segmento)) {
		free(buffer);
		return NULL; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
	}

	int numeroPagina = numero_pagina(ptr_segmento, direccion);
	int desplazamientoEnPagina = desplazamiento_en_pagina(ptr_segmento,
			direccion);

	int auxiliar = 0;
	int tamanioACopiar;
	int desplazamientoEnBuffer = 0;

	while (cantidadDeBytes > 0) {

		page* pagina = list_get(ptr_segmento->tabla_de_paginas,
				numeroPagina + auxiliar);
		frame* marco = pagina->frame;
		tamanioACopiar = minimo(cantidadDeBytes,
				(TAM_PAG - desplazamientoEnPagina));
		memcpy(buffer + desplazamientoEnBuffer,
				marco->memoria + desplazamientoEnPagina, tamanioACopiar);
		cantidadDeBytes -= tamanioACopiar;
		auxiliar++; //siguiente pagina
		desplazamientoEnBuffer += tamanioACopiar;
		desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
	}
	return (buffer);
}

int muse_cpy(t_proceso* proceso, t_list* paqueteRecibido) {

	int cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
	uint32_t direccion_pedida = *((uint32_t*) list_get(paqueteRecibido, 3));

	void* buffer_a_copiar = (void*)malloc(cantidad_de_bytes);

	memcpy(buffer_a_copiar, list_get(paqueteRecibido, 2), cantidad_de_bytes);

	segment* ptr_segmento = buscar_segmento_dada_una_direccion(
			proceso->tablaDeSegmentos, direccion_pedida);

	if (ptr_segmento == NULL) {
		free(buffer_a_copiar);
		return ERROR; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
	}

	if ((direccion_pedida + cantidad_de_bytes)
			> limite_segmento(ptr_segmento)) {
		free(buffer_a_copiar);
		return ERROR; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
	}

	if (direccion_pisa_alguna_metadata(ptr_segmento, direccion_pedida,
			cantidad_de_bytes)) {
		free(buffer_a_copiar);
		return ERROR;
	}

	int numeroPagina = numero_pagina(ptr_segmento, direccion_pedida);
	int desplazamientoEnPagina = desplazamiento_en_pagina(ptr_segmento,
			direccion_pedida);

	int auxiliar = 0;
	int tamanioACopiar;
	int desplazamientoEnBuffer = 0;

	while (cantidad_de_bytes > 0) {

		page* pagina = list_get(ptr_segmento->tabla_de_paginas,
				numeroPagina + auxiliar);
		frame* marco = pagina->frame;
		tamanioACopiar = minimo(cantidad_de_bytes,
				(TAM_PAG - desplazamientoEnPagina));

		memcpy(marco->memoria + desplazamientoEnPagina,
				buffer_a_copiar + desplazamientoEnBuffer, tamanioACopiar);

		cantidad_de_bytes -= tamanioACopiar;
		auxiliar++; //siguiente pagina
		desplazamientoEnBuffer += tamanioACopiar;
		desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
	}

	return 0;
}

int main(void) {
	inicilizar_tabla_de_frames();
	inicializar_bitmap();
	t_config* config = leer_config();
	void *memoria = (void*) malloc(leer_del_config("MEMORY_SIZE", config));
	dividir_memoria_en_frames(memoria, leer_del_config("PAGE_SIZE", config),
			leer_del_config("MEMORY_SIZE", config));
	TAM_PAG = leer_del_config("PAGE_SIZE", config);

	//Arranca a atender clientes

	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal para lectura de descriptores de fichero para select()
	int fdmax;        // Ultimo socket recibido
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	int socketEs = iniciar_socket_escucha("127.0.0.1",
			config_get_string_value(config, "LISTEN_PORT")); // obtener socket para listen

	FD_SET(socketEs, &master); // añadir socketEscucha al conjunto maestro
	fdmax = socketEs;

	while (1) {
		read_fds = master;
		select(fdmax + 1, &read_fds, NULL, NULL, NULL); // @suppress("Symbol is not resolved")
		for (int i = 0; i <= fdmax; i++) { // explorar conexiones existentes en busca de datos que leer
			if (FD_ISSET(i, &read_fds)) { //Hay datos que leer...
				if (i == socketEs) { //si se recibe en el socket escucha hay nuevas conexiones que aceptar
					admitirNuevoCliente(&master, &fdmax, i); //agregar al master los nuevos clientes
				} else {
					atenderCliente(&master, i); //leer mensajes y paquetes e imprimirlos por consola
				}
			}
		}
	}

	liberacion_de_recursos(memoria, config);

	return 0;
}

