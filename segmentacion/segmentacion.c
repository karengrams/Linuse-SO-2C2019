#include "segmentacion.h"
#include "../paginacion/paginacion.h"
//#include "../paginacion/frames.h"

// rompe por alguna razon

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

int posicion_en_tabla_segmento(segment* elemento, t_list* tabla_de_segmentos){
	segment *comparador = (segment*)malloc(sizeof(segment));
	for(int index = 0 ; index < tabla_de_segmentos->elements_count; index++){
		comparador = list_get(tabla_de_segmentos, index);
		if (memcmp(elemento, comparador, sizeof(segment)) == 0) {//Si son iguales devuelve 0
			free(comparador);
			return index;
		}
	}
	free(comparador);
	return -1;
}

// te quedaste aca loca


bool segmento_puede_agrandarse(segment* segmento, t_list* tabla_de_segmentos, int valorPedido){
	int pos_seg=posicion_en_tabla_segmento(segmento,tabla_de_segmentos);
	segment *siguiente=(segment*)list_get(tabla_de_segmentos, (pos_seg+1));
	int paginasNecesarias = paginas_necesarias(valorPedido);
	bool respuesta;
	if(siguiente->baseLogica-limite_segmento(segmento) >= (paginasNecesarias * TAM_PAG))
		return true;
	free(siguiente);
	return false;
}


bool segmento_tiene_espacio(segment* segmento, int tamanio, uint32_t* direccionConEspacioLibre){
	void *segmentoMappeado = malloc(((segmento->tablaDePaginas->elements_count)*tamanio_paginas())+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado);
	bool respuesta = tiene_espacio(segmentoMappeado, tamanio, direccionConEspacioLibre);
	free(segmentoMappeado);
	return respuesta;
}

void mappear_segmento(segment* segmento, void* segmentoMappeado){
	int numeroDePagina = 0;
	metadata *finDeSegmento = malloc(sizeof(metadata));
	finDeSegmento->bytes = -1;
	finDeSegmento->ocupado = false; //metadata que agrego para saber donde termina el segmento
	int desplazamiento = 0;
	page *pagina = malloc(sizeof(page));
	while(numeroDePagina < list_size((segmento->tablaDePaginas))){ //Copio todos los marcos en el mappeo

		pagina = (page*)list_get((segmento->tablaDePaginas), numeroDePagina);
		memcpy(segmentoMappeado + desplazamiento, (list_get(FRAMES_TABLE, (pagina->numero_frame))), tamanio_paginas());
		desplazamiento += tamanio_paginas();
		numeroDePagina ++;
	}

	memcpy(segmentoMappeado + desplazamiento, finDeSegmento, sizeof(metadata));
	free(pagina);
	free(finDeSegmento);
}



bool tiene_espacio(void* punteroAMemoria, int valorPedido, uint32_t* direccion){
	metadata *metadat = malloc(sizeof(metadata)); // Deberia ser:( metadata*)malloc(sizeof(metadata))
	uint32_t desplazamiento = 0;

	memcpy(metadat, punteroAMemoria, sizeof(metadata));

	while(metadat->bytes != -1){ //En el fin de segmento mapeado le agrego una metadata con bytes = -1

		if(!(metadat->ocupado)){ //si no esta ocupado pregunto si cabe el valor pedido
			if((metadat->bytes == valorPedido) || (metadat->bytes >= (valorPedido+5)))
				//Si el espacio libre es exactamente igual al valor pedido o si es mayorIgual al
				//valor pedido mas el valor de la metadata que iria luego...
				*direccion = desplazamiento; //Guardamos en el puntero la direccion al inicio de la metadata
				return true;
		}

		desplazamiento += sizeof(metadata);
		desplazamiento += metadat->bytes; //Nos movemos a la siguiente metadata
		memcpy(metadat, punteroAMemoria + desplazamiento, sizeof(metadata));

	}
		free(metadat);
		return false;
}

div_t numero_pagina(segment* segmento, uint32_t direccion){
	int desplazamientoEnSegmento = direccion - segmento->baseLogica;
	return div(desplazamientoEnSegmento, tamanio_paginas());
}

int tamanio_segmento(segment* segmento){
	return ((list_size(segmento->tablaDePaginas))*(tamanio_paginas()));
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

void desmappear_segmento(segment* segmento, void* segmentoMappeado){
	int numeroDePagina = 0;
	int desplazamiento = 0;
	page *pagina = malloc(sizeof(page));

	while(numeroDePagina < list_size((segmento->tablaDePaginas))){ //Copio todos los marcos en el mappeo

		pagina = (page*)list_get((segmento->tablaDePaginas), numeroDePagina);
		memcpy((list_get(FRAMES_TABLE, (pagina->numero_frame))), segmentoMappeado + desplazamiento, tamanio_paginas());
		desplazamiento += tamanio_paginas();
		numeroDePagina ++;
	}

	free(pagina);

}


void reservar_memoria(int bytesPedidos, uint32_t desplazamiento, segment* segmento){ //Segmento con lugar, desplazamiento en donde tiene ese lugar

	void *segmentoMappeado = malloc(((segmento->tablaDePaginas->elements_count)*tamanio_paginas())+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado);
	metadata* metadataAux = malloc(sizeof(metadata));

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

