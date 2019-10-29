#include <stdlib.h>
#include "utils.h"

int ERROR = -1;

int min(int a, int b){
	if(a<b)
		return a;
	 else
		return b;
}

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

int iniciar_proceso(t_list* tablaProcesos, t_proceso* proceso, t_list* paquete, char* ip){

			if(proceso != NULL)
           	   	return -1; //YA EXISTE EN NUESTRA TABLA: ERROR
			proceso = crear_proceso(*((int*)paquete->head->data), ip);
           	list_add(tablaProcesos, proceso);// Si no existe lo agregamos
           	return 0;
}

void liberar_proceso(t_proceso* proceso, t_list* tablaProcesos){
	list_remove(tablaProcesos, posicion_en_lista_proceso(proceso, tablaProcesos));
	liberar_segmentos(proceso->tablaDeSegmentos);
	free(proceso->ip);
	free(proceso);
}

//muse_malloc(t_proceso* procesoAAtender, t_list* paquete){
//	uint32_t tamanioPedido = list_get(paquete,1);
//
//	if((procesoAAtender->tablaDeSegmentos->elements_count) == 0){//Si no tiene ningun segmento le creamos uno
//		uint32_t tamanioSegmento = paginas_necesarias(tamanioPedido)*tamanio_paginas();
//		t_list* listaPaginas = crear_lista_paginas(paginas_necesarias(tamanioPedido));
//		t_segmento* segmentoNuevo = crear_segmento(HEAP, 0, tamanioSegmento, listaPaginas);
//		list_add((procesoAAtender->tablaDeSegmentos), segmentoNuevo);
//	}
//	//chequear si hay espacio en algun segmento heap
//	//si no hay,
//	//chequear si se puede extender algun segmento heap
//	//si no se puede,
//	//crear un nuevo segmento heap
//
//
//}




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
				marco = list_get(FRAMES_TABLE, (pagina->numero_frame)); //ahora me marca la lista de marcos, pero la vamos a hacer globar so doesn't matter

				memcpy(buffer+desplazamientoEnBuffer, marco+desplazamientoEnPagina , tamanioACopiar);

				cantidadDeBytes -= tamanioACopiar;
				auxiliar++; //siguiente pagina
				desplazamientoEnBuffer += tamanioACopiar;
				desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
		}

		free(pagina);
		return(buffer);
}

int magia_muse_init(t_proceso* cliente_a_atender){

		if(cliente_a_atender != NULL){
			return ERROR; //YA EXISTE EN NUESTRA TABLA ERROR

			} else {
			list_add(PROCESS_TABLE, cliente_a_atender); //Si no existe lo agregamos
			return 0;
			}
}


