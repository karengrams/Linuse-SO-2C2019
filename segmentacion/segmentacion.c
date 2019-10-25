#include "segmentacion.h"
#include "../paginacion/paginacion.h"
#include "../paginacion/paginacion.c"
#include "../paginacion/frames.h"


segment* crear_segmento(segment_type tipo, uint32_t baseLogica, int tamanio, t_list* listaPaginas){

//En lugar de mandarle la lista de paginas ya creada se le podria pasar la cantidad de paginas,
//o el tamanio que deberia tener el segmento y crearle las paginas aca adentro, meh.

	segment* segmento = (segment*)malloc(sizeof(segment));
	segmento->baseLogica = baseLogica;
	segmento->tamanio = tamanio;
	segmento->tipo = tipo;
	segmento->tablaDePaginas = listaPaginas;
	return segmento;
}

int posicion_en_tabla_de_segmentos(segment* elemento){

	segment* comparador = (segment*)malloc(sizeof(segment));

	for(int index = 0 ; index < SEGMENT_TABLE->elements_count; index++){
		comparador = list_get(SEGMENT_TABLE, index);
		if (memcmp(elemento, comparador, sizeof(segment)) == 0) {//Si son iguales devuelve 0
			free(comparador);
			return index;
		}
	}

	free(comparador);

	return -1; //Si no esta devuelve -1
}

void liberar_segmentos(t_list* segmentos){
	//Ver aca que onda... como destruir esta lista va a cambiar dependiendo si es
	//Heap o mmap (creo)
}

bool segmento_tiene_espacio(segment* segmento, int tamanio, t_list* listaDeMarcos){
	void* segmentoMappeado = (void*)malloc((segmento->tamanio)+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado, listaDeMarcos);
	bool respuesta = tiene_espacio(segmentoMappeado, tamanio);
	free(segmentoMappeado);
	return respuesta;
}

void mappear_segmento(segment* segmento, void* segmentoMappeado, t_list* listaDeMarcos){
	int numeroDePagina = 0;
	metadata* finDeSegmento = (metadata*)malloc(sizeof(metadata));
	finDeSegmento->bytes = -1;
	finDeSegmento->ocupado = false; //metadata que agrego para saber donde termina el segmento
	int desplazamiento = 0;
	t_pagina* pagina = malloc(sizeof(t_pagina));
	while(numeroDePagina < list_size((segmento->tablaDePaginas))){ //Copio todos los marcos en el mappeo

		pagina = (t_pagina*)list_get((segmento->tablaDePaginas), numeroDePagina);
		memcpy(segmentoMappeado + desplazamiento, (list_get(listaDeMarcos, (pagina->numero_frame))), tamanio_paginas());
		desplazamiento += tamanio_paginas();
		numeroDePagina ++;
	}

	memcpy(segmentoMappeado + desplazamiento, finDeSegmento, sizeof(metadata));
	free(pagina);
	free(finDeSegmento);
}

bool segmento_puede_agrandarse(segment* segmento, t_list* listaDeSegmentos, int valorPedido){
	segment* siguiente = (segment*)malloc(sizeof(segment));
	bool respuesta;
	int paginasNecesarias = paginas_necesarias(valorPedido);
	int posicion = posicion_en_lista_segmento(segmento, listaDeSegmentos);
	siguiente = list_get(listaDeSegmentos, (posicion+1));

	if(((siguiente->baseLogica)-(segmento->baseLogica)-(segmento->tamanio))>=(paginasNecesarias * tamanio_paginas())){
		free(siguiente);
		return true;
	}

	free(siguiente);
	return false;
}

bool tiene_espacio(void* punteroAMemoria, int valorPedido){
	metadata* metadata = malloc(sizeof(metadata)); // Deberia ser:( metadata*)malloc(sizeof(metadata))
	int desplazamiento = 0;

	memcpy(metadata, punteroAMemoria, sizeof(metadata));

	while(metadata->bytes != -1){ //En el fin de segmento mapeado le agrego una metadata con bytes = -1

		if(!(metadata->ocupado)){ //si no esta ocupado pregunto si cabe el valor pedido
			if(metadata->bytes >= valorPedido)
				return true;
		}

		desplazamiento += sizeof(metadata);
		desplazamiento += metadata->bytes; //Nos movemos a la siguiente metadata
		memcpy(metadata, punteroAMemoria + desplazamiento, sizeof(metadata));

	}
		free(metadata);
		return false;
}


