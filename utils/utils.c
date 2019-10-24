#include <stdlib.h>
#include "utils.h"


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

void iniciar_proceso(t_list* tablaProcesos, t_proceso* proceso, t_list* paquete, char* ip){

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

