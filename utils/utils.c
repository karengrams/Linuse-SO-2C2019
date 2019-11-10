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

t_proceso* crear_proceso(int id, char* ip) {
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->ip = malloc(sizeof(char) * 16);
	strcpy(proceso->ip, ip);
	proceso->tablaDeSegmentos = list_create();
	return proceso;
}

t_proceso* buscar_proceso(t_list* paqueteRecibido, char* ipCliente) {
	int id = *((int*) list_get(paqueteRecibido, 0));

	bool mismoipid(void* arg) {
		t_proceso* cliente = (t_proceso*) arg;
		return ((cliente->id) == id && !(strcmp(ipCliente, cliente->ip)));
	}

	return list_find(PROCESS_TABLE, mismoipid);

}

int posicion_en_lista_proceso(t_proceso* elemento) {
	t_proceso* comparador = malloc(sizeof(t_proceso));

	for (int index = 0; index < PROCESS_TABLE->elements_count; index++) {
		comparador = list_get(PROCESS_TABLE, index);
		if (memcmp(elemento, comparador, sizeof(t_proceso)) == 0) { //Si son iguales devuelve 0
			free(comparador);
			return index;
		}
	}
	free(comparador);
	return -1; //Si no esta devuelve -1
}

void liberar_proceso(t_proceso* proceso) {
	list_remove(PROCESS_TABLE, posicion_en_lista_proceso(proceso));
	//liberar_segmentos(proceso->tablaDeSegmentos);
	free(proceso->ip);
	free(proceso);
}
