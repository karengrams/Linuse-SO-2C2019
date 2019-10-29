#include "segmentacion.h"
#include "../paginacion/paginacion.h"
#include "../paginacion/paginacion.c"
#include "../paginacion/frames.h"


segment* crear_segmento(segment_type tipo, uint32_t baseLogica, int tamanio, t_list* listaPaginas){

//En lugar de mandarle la lista de paginas ya creada se le podria pasar la cantidad de paginas,
//o el tamanio que deberia tener el segmento y crearle las paginas aca adentro, meh.

	segment *segmento = malloc(sizeof(segment));
	segmento->baseLogica = baseLogica;
	segmento->tamanio = tamanio;
	segmento->tipo = tipo;
	segmento->tablaDePaginas = listaPaginas;
	return segmento;
}

int posicion_en_tabla_de_segmentos(segment* elemento, t_list* tabla_de_segmentos){

	segment *comparador = malloc(sizeof(segment));

	for(int index = 0 ; index < tabla_de_segmentos->elements_count; index++){
		comparador = list_get(tabla_de_segmentos, index);
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
	void *segmentoMappeado = malloc((segmento->tamanio)+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado, listaDeMarcos);
	bool respuesta = tiene_espacio(segmentoMappeado, tamanio);
	free(segmentoMappeado);
	return respuesta;
}

void mappear_segmento(segment* segmento, void* segmentoMappeado, t_list* listaDeMarcos){
	int numeroDePagina = 0;
	metadata *finDeSegmento = malloc(sizeof(metadata));
	finDeSegmento->bytes = -1;
	finDeSegmento->ocupado = false; //metadata que agrego para saber donde termina el segmento
	int desplazamiento = 0;
	page *pagina = malloc(sizeof(page));
	while(numeroDePagina < list_size((segmento->tablaDePaginas))){ //Copio todos los marcos en el mappeo

		pagina = (page*)list_get((segmento->tablaDePaginas), numeroDePagina);
		memcpy(segmentoMappeado + desplazamiento, (list_get(listaDeMarcos, (pagina->numero_frame))), tamanio_paginas());
		desplazamiento += tamanio_paginas();
		numeroDePagina ++;
	}

	memcpy(segmentoMappeado + desplazamiento, finDeSegmento, sizeof(metadata));
	free(pagina);
	free(finDeSegmento);
}


bool segmento_puede_agrandarse(segment* segmento, t_list* listaDeSegmentos, int valorPedido){
	segment *siguiente = malloc(sizeof(segment));
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
	metadata *metadata = malloc(sizeof(metadata)); // Deberia ser:( metadata*)malloc(sizeof(metadata))
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



uint32_t limite_segmento(segment* segmento){
	return ((segmento->baseLogica)+list_size(segmento->tablaDePaginas))*tamanio_paginas();
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


