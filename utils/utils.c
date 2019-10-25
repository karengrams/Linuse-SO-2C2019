#include <stdlib.h>
#include "utils.h"

int ERROR = -1;

t_proceso* crear_proceso(int id, char* ip){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->ip = malloc(sizeof(char)*16);
	strcpy(proceso->ip, ip);
	proceso->tablaDeSegmentos = list_create();
	return proceso;
}


t_proceso* buscar_proceso(t_list* paqueteRecibido, t_list* tablaDeProcesos, char* ipCliente){
	int id = *((int*)list_get(paqueteRecibido, 0));

	bool mismoipid(void* arg){
		t_proceso* cliente = (t_proceso*)arg;
		return ((cliente->id) == id && !(strcmp(ipCliente, cliente->ip)));
	}

	return list_find(tablaDeProcesos, mismoipid);

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
