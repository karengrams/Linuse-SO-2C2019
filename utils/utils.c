#include <stdlib.h>
#include "utils.h"

int ERROR = -1;
/*
int minimo(int a, int b){
	if(a<b)
		return a;
	 else
		return b;
}*/

t_proceso* crear_proceso(int id, char* ip){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->ip = malloc(sizeof(char)*16);
	strcpy(proceso->ip, ip);
	proceso->tablaDeSegmentos = list_create();
	return proceso;
}

t_proceso* buscar_proceso(t_list* paqueteRecibido, char* ipCliente){
	int id = *((int*)list_get(paqueteRecibido, 0));

	bool mismoipid(void* arg){
		t_proceso* cliente = (t_proceso*)arg;
		return ((cliente->id) == id && !(strcmp(ipCliente, cliente->ip)));
	}

	return list_find(PROCESS_TABLE, mismoipid);

}

int posicion_en_lista_proceso(t_proceso* elemento, t_list* lista){
	t_proceso* comparador = malloc(sizeof(t_proceso));

	for(int index = 0 ; index < lista->elements_count; index++){
		comparador = list_get(lista, index);
		if (memcmp(elemento, comparador, sizeof(t_proceso)) == 0) {//Si son iguales devuelve 0
			free(comparador);
			return index; }
	}
	free(comparador);
	return -1; //Si no esta devuelve -1
}

void liberar_proceso(t_proceso* proceso, t_list* tablaProcesos){
	list_remove(tablaProcesos, posicion_en_lista_proceso(proceso, tablaProcesos));
	liberar_segmentos(proceso->tablaDeSegmentos);
	free(proceso->ip);
	free(proceso);
}

uint32_t magia_muse_alloc(t_proceso* proceso, int tam){
	uint32_t offset;
	t_list *ptr_aux = list_duplicate(proceso->tablaDeSegmentos);
	bool condicion = false;
	if(proceso->tablaDeSegmentos!=NULL){
		//Situacion nro 2. (o de expansion)
		while(ptr_aux->head!=NULL){
		segment* segmento = (segment*)list_find(ptr_aux,(void*)es_segmento_de_tipo_HEAP);
		if(segmento_tiene_espacio_disponible(segmento,tam,&offset)){
			condicion=true;
			(*segmento).tamanio=(*segmento).tamanio+tam;
			reservar_memoria(tam,offset,segmento);
		}
		if(segmento_puede_agrandarse(segmento,proceso->tablaDeSegmentos,tam)){
			condicion=true;
		}
			list_remove(ptr_aux,posicion_de_segmento(segmento,ptr_aux));
		}
	}
	else{
		//Situacion nro 1. (o de creacion de segmento)
		// No hay ningun segmento de tipo HEAP que estamos buscando
		int cantidad_de_paginas= paginas_necesarias(tam);
		segment *segmento = crear_segmento(HEAP,0,tam); // TODO: implementar de forma correcta la base
		list_add(proceso->tablaDeSegmentos,segmento); // Se agrega el segmento creado a nuestra tabla de segmentos
		(*segmento).tablaDePaginas=crear_lista_paginas(cantidad_de_paginas);
		asignar_marcos((*segmento).tablaDePaginas);
		return (*segmento).baseLogica+sizeof(metadata); /*Devuelve la base logica + 5 de la metadata.
		Como no hay ningun segmento que se pueda extender, o no exista ningun segmento
		se devuelve 'automaticamente' 5. TODO: fijarse si funciona correctamente */
	}

}

void* magia_muse_get(t_proceso* proceso, t_list* paqueteRecibido){
	int cantidadDeBytes = *((int*)list_get(paqueteRecibido, 1));
	uint32_t direccion = *((uint32_t*)list_get(paqueteRecibido, 2));

	segment* segmento = buscar_segmento_dada_una_direccion(proceso->tablaDeSegmentos, direccion);
	void* buffer = malloc(cantidadDeBytes);
	void* marco;
	page* pagina = malloc(sizeof(page));

		if(segmento == NULL){
			free(buffer);
			return NULL; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
		}

		if((direccion+cantidadDeBytes)>limite_segmento(segmento)){
			free(buffer);
			return NULL; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
		}

		div_t aux = numero_pagina(segmento, direccion);
		int numeroPagina = aux.quot; //pagina correspondiente a la direccion
		int desplazamientoEnPagina = aux.rem; //desde que posicion de esa pagina vamos a empezar a copiar

		//int sobrante = cantidadDeBytes - (tamanio_paginas() - desplazamientoEnPagina);
		//int cantidadDePaginasAMappear = 1; //minimo vamos a mappear la pagina que tiene el dato
		//
		//		if(sobrante>0){
		//			cantidadDePaginasAMappear += paginas_necesarias(sobrante);
		//			} CREO que esto termina siendo innecesario, lo dejo comentado por las dudas

		int auxiliar = 0;
		int tamanioACopiar;
		int desplazamientoEnBuffer = 0;

		while(cantidadDeBytes>0){

				pagina = list_get(segmento->tablaDePaginas, numeroPagina+auxiliar);
				tamanioACopiar = min(cantidadDeBytes, (tamanio_paginas() - desplazamientoEnPagina));
				marco = list_get(FRAMES_TABLE, (pagina->numero_frame));

				memcpy(buffer+desplazamientoEnBuffer, marco+desplazamientoEnPagina , tamanioACopiar);

				cantidadDeBytes -= tamanioACopiar;
				auxiliar++; //siguiente pagina
				desplazamientoEnBuffer += tamanioACopiar;
				desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
		}

		free(pagina);
		return(buffer);
}

int magia_muse_init(t_proceso* cliente_a_atender, char* ipCliente, int id){
	PROCESS_TABLE=list_create();
		if(cliente_a_atender != NULL){
			return ERROR; //YA EXISTE EN NUESTRA TABLA ERROR

			} else {
			t_proceso* procesoNuevo = crear_proceso(id, ipCliente);
			list_add(PROCESS_TABLE, procesoNuevo); //Si no existe lo creamos y agregamos
			return 0;
			}
}

int magia_muse_cpy(t_proceso* proceso, t_list* paqueteRecibido){

	int cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 1));
	uint32_t direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 3));

	void* buffer = (void*)malloc(cantidad_de_bytes);

	memcpy(buffer, list_get(paqueteRecibido,2), cantidad_de_bytes);

	segment* segmento = buscar_segmento_dada_una_direccion(proceso->tablaDeSegmentos, direccion_pedida);

		if(segmento == NULL){
				free(buffer);
				return -1; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
			}

		if((direccion_pedida+cantidad_de_bytes)>limite_segmento(segmento)){
				free(buffer);
				return -1; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
			}

	int desplazamientoEnSegmento = direccion_pedida - segmento->baseLogica;
	void *segmentoMappeado = malloc(((segmento->tablaDePaginas->elements_count)*tamanio_paginas())+sizeof(metadata));
	mappear_segmento(segmento, segmentoMappeado);

	bool puedeEscribirse = segmento_puede_escribirse(segmentoMappeado, desplazamientoEnSegmento, cantidad_de_bytes);

	if(!puedeEscribirse){
		free(buffer);
		free(segmentoMappeado);
		return -1; //Quiere escribir sobre una metadata.
	}

	escribir_segmento(segmento, direccion_pedida, cantidad_de_bytes, buffer);
	free(buffer);
	free(segmentoMappeado);
	return 0;
}


