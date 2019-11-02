#include "segmentacion.h"
#include "../paginacion/paginacion.h"
#include "../paginacion/frames.h"
#include "../utils/utils.h"

int minimo(int a, int b){
	if(a<b)
		return a;
	 else
		return b;
}

bool es_segmento_de_tipo_HEAP(segment *segmento){
	return segmento->tipo == HEAP;
}

uint32_t limite_segmento(segment* segmento){
	if(segmento->tablaDePaginas!=NULL)
	return (segmento->baseLogica)+(segmento->tablaDePaginas->elements_count)*TAM_PAG;
	else
		return (segmento->baseLogica);
}

segment* crear_segmento(segment_type tipo, uint32_t baseLogica,int tam){
	segment *segmento_ptr = (segment*)malloc(sizeof(segment));
	(*segmento_ptr).baseLogica = baseLogica;
	(*segmento_ptr).tamanio = tam;
	(*segmento_ptr).tipo = tipo;
	(*segmento_ptr).tablaDePaginas = NULL;
	return segmento_ptr;
}

// TODO: Cuando se tratan de dos segmentos de igual tam. los toma como iguales. Ver de hacer de otra forma
int posicion_en_tabla_segmento(segment* elemento, t_list* tabla_de_segmentos){
	segment *comparador = (segment*)malloc(sizeof(segment));
	for(int index = 0 ; index < tabla_de_segmentos->elements_count; index++){
		comparador = list_get(tabla_de_segmentos, index);
		if (!memcmp(elemento, comparador, sizeof(segment))) {//Si son iguales devuelve 0
			free(comparador);
			return index;
		}
	}
	free(comparador);
	return -1;
}

// TODO:Habria que probar si funciona cuando haya un MMAP adelante
bool segmento_puede_agrandarse(segment* segmento, t_list* tabla_de_segmentos, int valorPedido){
	int pos_seg=posicion_en_tabla_segmento(segmento,tabla_de_segmentos);
	segment *siguiente=(segment*)list_get(tabla_de_segmentos, (pos_seg+1));
	int paginasNecesarias = paginas_necesarias(valorPedido);
	bool respuesta;
	if(siguiente->baseLogica-limite_segmento(segmento) >= (paginasNecesarias * TAM_PAG)){
		free(siguiente);
		return true;
	}
	free(siguiente);
	return false;
}

div_t numero_pagina(segment* segmento, uint32_t direccion){
	int desplazamientoEnSegmento = direccion - segmento->baseLogica;
	return div(desplazamientoEnSegmento, TAM_PAG);
}

//TODO: ver caso de error
int tamanio_segmento(segment* segmento){
	return ((list_size(segmento->tablaDePaginas))*(TAM_PAG));
}

bool segmento_tiene_espacio_disponible(segment* segmento, int tamanio,uint32_t* direccionConEspacioLibre){
	void *segmentoMappeado = (void*)malloc(((segmento->tablaDePaginas->elements_count)*TAM_PAG)+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado);
	bool respuesta = tiene_espacio(segmentoMappeado,tamanio,direccionConEspacioLibre);
	free(segmentoMappeado);
	return respuesta;
}

void mappear_segmento(segment* segmento, void* mem){
	int numeroDePagina = 0;
	metadata *fin_seg_metadata = (metadata*)malloc(sizeof(metadata));
	(*fin_seg_metadata).bytes = -1;
	(*fin_seg_metadata).ocupado = false; //metadata que agrego para saber donde tera el segmento
	int desplazamiento = 0;
	page *pagina;
	while(numeroDePagina < list_size((segmento->tablaDePaginas))){ //Copio todos los marcos en el mappeo
		pagina = (page*)list_get((segmento->tablaDePaginas), numeroDePagina);
		void *frame= (void*)list_get(FRAMES_TABLE, (pagina->numero_frame));
		memcpy(mem + desplazamiento,frame, TAM_PAG);
		desplazamiento += TAM_PAG;
		numeroDePagina ++;
	}
	memcpy(mem + desplazamiento, fin_seg_metadata, sizeof(metadata));
	free(pagina);
	free(fin_seg_metadata);
}

bool tiene_espacio(void* punteroAMemoria, int valorPedido, uint32_t* direccion){
	metadata *ptr_metadata = (metadata*)malloc(sizeof(metadata)); // Deberia ser:( metadata*)malloc(sizeof(metadata))
	uint32_t desplazamiento = 0;

	memcpy(ptr_metadata, punteroAMemoria, sizeof(metadata));

	while(ptr_metadata->bytes != -1){ //En el fin de segmento mapeado le agrego una metadata con bytes = -1

		if(!(ptr_metadata->ocupado)){ //si no esta ocupado pregunto si cabe el valor pedido
			if((ptr_metadata->bytes == valorPedido) || (ptr_metadata->bytes >= (valorPedido+5)))
				//Si el espacio libre es exactamente igual al valor pedido o si es mayorIgual al
				//valor pedido mas el valor de la metadata que iria luego...
				*direccion = desplazamiento; //Guardamos en el puntero la direccion al inicio de la metadata
				return true;
		}

		desplazamiento += sizeof(metadata);
		desplazamiento += ptr_metadata->bytes; //Nos movemos a la siguiente metadata
		memcpy(ptr_metadata, punteroAMemoria + desplazamiento, sizeof(metadata));

	}
		free(ptr_metadata);
		return false;
}

void desmappear_segmento(segment* segmento, void* segmentoMappeado){
	int numeroDePagina = 0;
	int desplazamiento = 0;
	page *pagina = malloc(sizeof(page));

	while(numeroDePagina < list_size((segmento->tablaDePaginas))){ //Copio todos los marcos en el mappeo

		pagina = (page*)list_get((segmento->tablaDePaginas), numeroDePagina);
		memcpy((list_get(FRAMES_TABLE, (pagina->numero_frame))), segmentoMappeado + desplazamiento, TAM_PAG);
		desplazamiento += TAM_PAG;
		numeroDePagina ++;
	}

	free(pagina);

}

segment* buscar_segmento_dada_una_direccion(t_list* tablaSegmentos, uint32_t direccion){
	segment* segmentoEncontrado = malloc(sizeof(segment));

	for(int i = 0; i<(tablaSegmentos->elements_count); i++){
		segmentoEncontrado = list_get(tablaSegmentos, i);

			if((segmentoEncontrado->baseLogica>direccion) || (segmentoEncontrado == NULL)){ //si ya nos pasamos devolvemos error (baselogica = -1)
				return NULL;
				}

			if((limite_segmento(segmentoEncontrado))>direccion){
				return segmentoEncontrado;
			}

	}
	return NULL; //Por las dudas de que me este olvidando de algun caso
}

void reservar_memoria(int bytesPedidos, uint32_t desplazamiento, segment* segmento){ //Segmento con lugar, desplazamiento en donde tiene ese lugar

	void *segmentoMappeado = (void*)malloc(((segmento->tablaDePaginas->elements_count)*TAM_PAG)+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado);
	metadata* metadataAux = (metadata*)malloc(sizeof(metadata));

	//trabajo con el segmento mappeado y luego lo vuelco en los marcos
	memcpy(metadataAux, segmentoMappeado+desplazamiento, sizeof(metadata));

	if(metadataAux->bytes == bytesPedidos){ //Si es igual lo que hay libre y lo que pidio, solamente lo ponemos ocupado
		metadataAux->ocupado = true;
		memcpy(segmentoMappeado+desplazamiento, metadataAux, sizeof(metadata));

	} else { //Deberia tener lo pedido y sobrarle al menos 5 bytes para la siguiente metadata...
		int nuevosBytesLibres = metadataAux->bytes-bytesPedidos-sizeof(metadata);
		metadataAux->ocupado = true;
		metadataAux->bytes = bytesPedidos;
		memcpy(segmentoMappeado+desplazamiento, metadataAux, sizeof(metadata));
		//Le cargo los valores de la metadata que va despues del espacio reservado
		metadataAux->ocupado = false;
		metadataAux->bytes = nuevosBytesLibres;
		memcpy(segmentoMappeado+desplazamiento+sizeof(metadata)+bytesPedidos, metadataAux, sizeof(metadata));
	}

	desmappear_segmento(segmento, segmentoMappeado);
	free(metadataAux);
	free(segmentoMappeado);
}

//FUNCION AUXILIAR DE RESERVAR_MEMORIA(..)
void escribir_segmento(segment* segmento, uint32_t direccion_pedida, int cantidad_de_bytes, void* buffer){
	div_t aux = numero_pagina(segmento, direccion_pedida);
	int numeroPagina = aux.quot; //pagina correspondiente a la direccion
	int desplazamientoEnPagina = aux.rem; //desde que posicion de esa pagina vamos a empezar a copiar

	page* pagina = malloc(sizeof(page));
	void* marco;

	int auxiliar = 0;
	int tamanioACopiar;
	int desplazamientoEnBuffer = 0;

	while(cantidad_de_bytes>0){

			pagina = list_get(segmento->tablaDePaginas, numeroPagina+auxiliar);
			int nro = TAM_PAG - desplazamientoEnPagina;
			tamanioACopiar = minimo(cantidad_de_bytes, nro);
			marco = list_get(FRAMES_TABLE, (pagina->numero_frame));

			memcpy(marco+desplazamientoEnPagina, buffer+desplazamientoEnBuffer, tamanioACopiar);

			cantidad_de_bytes -= tamanioACopiar;
			auxiliar++; //siguiente pagina
			desplazamientoEnBuffer += tamanioACopiar;
			desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
			}

	free(pagina);
}

bool segmento_puede_escribirse(void* segmentoMappeado, int desplazamientoEnSegmento, int cantidad_de_bytes){
	metadata* metadataAux = malloc(sizeof(metadata));
	int desplazamiento = 0;

	memcpy(metadataAux, segmentoMappeado+desplazamiento, sizeof(metadata));

	while(metadataAux->bytes != -1){

		desplazamiento += sizeof(metadata); //Limite inferior del lugar disponible
		int bytesDisponibles = metadataAux->bytes + desplazamiento; //Limite superior del lugar Disponible

			if(desplazamientoEnSegmento<desplazamiento) //ya nos pasamos (puede ser que haya pedido la direccion 0 del segmento por ejemplo
				return false;

			if((desplazamientoEnSegmento+cantidad_de_bytes)<=bytesDisponibles)
				return true;

		desplazamiento = bytesDisponibles;
		memcpy(metadataAux, segmentoMappeado+desplazamiento, sizeof(metadata));
	}

	free(metadataAux);
	return false; //por si me olvide de algun caso medio borde
}

