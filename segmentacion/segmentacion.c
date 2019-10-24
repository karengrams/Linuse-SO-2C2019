#include "segmentacion.h"

t_segmento* crear_segmento(int type, uint32_t baseLogica, int tamanio, t_list* listaPaginas){ 

//En lugar de mandarle la lista de paginas ya creada se le podria pasar la cantidad de paginas,
//o el tamanio que deberia tener el segmento y crearle las paginas aca adentro, meh.

	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->baseLogica = baseLogica;
	segmento->tamanio = tamanio;
	segmento->tipo = type;
	segmento->tablaDePaginas = listaPaginas;
	return segmento;
}

int posicion_en_lista_segmento(t_segmento* elemento, t_list* lista){
	t_segmento* comparador = malloc(sizeof(t_segmento));
	for(int index = 0 ; index < lista->elements_count; index++){
		comparador = list_get(lista, index);
		if (memcmp(elemento, comparador, sizeof(t_segmento)) == 0) {//Si son iguales devuelve 0
			free(comparador);
			return index; }
	}
	free(comparador);
	return -1; //Si no esta devuelve -1
}

void liberar_segmentos(t_list* segmentos){
	//Ver aca que onda... como destruir esta lista va a cambiar dependiendo si es
	//Heap o mmap (creo)
}

bool segmento_tiene_espacio(t_segmento* segmento, int tamanio, t_list* listaDeMarcos){
	void* segmentoMappeado = malloc((segmento->tamanio)+sizeof(t_metadata));
	mappear_segmento(segmento, segmentoMappeado, listaDeMarcos);
	bool respuesta = tiene_espacio(segmentoMappeado, tamanio);
	free(segmentoMappeado);
	return respuesta;
}

void mappear_segmento(t_segmento* segmento, void* segmentoMappeado, t_list* listaDeMarcos){
	int numeroDePagina = 0;
	t_metadata* finDeSegmento = malloc(sizeof(t_metadata));
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

	memcpy(segmentoMappeado + desplazamiento, finDeSegmento, sizeof(t_metadata));
	free(pagina);
	free(finDeSegmento);
}

bool segmento_puede_agrandarse(t_segmento* segmento, t_list* listaDeSegmentos, int valorPedido){
	t_segmento* siguiente = malloc(sizeof(t_segmento));
	bool respuesta;
	int paginasNecesarias = paginas_necesarias(valorPedido);
	int posicion = posicion_en_lista_segmento(segmento, listaDeSegmentos);
	siguiente = list_get(listaDeSegmentos, (posicion+1));

	if(((siguiente->baseLogica)-(segmento->baseLogica)-(segmento->tamanio))>=(paginasNecesarias * tamanio_paginas())){

		free(siguiente);
		return true;}

	free(siguiente);
	return false;
}

bool tiene_espacio(void* punteroAMemoria, int valorPedido){
	t_metadata* metadata = malloc(sizeof(t_metadata));
	int desplazamiento = 0;

	memcpy(metadata, punteroAMemoria, sizeof(t_metadata));

	while(metadata->bytes != -1){ //En el fin de segmento mapeado le agrego una metadata con bytes = -1

		if(!(metadata->ocupado)){ //si no esta ocupado pregunto si cabe el valor pedido
			if(metadata->bytes >= valorPedido)
				return true;
		}

		desplazamiento += sizeof(t_metadata);
		desplazamiento += metadata->bytes; //Nos movemos a la siguiente metadata
		memcpy(metadata, punteroAMemoria + desplazamiento, sizeof(t_metadata));

	}
		free(metadata);
		return false;
}


