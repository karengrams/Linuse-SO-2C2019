/*
 * suse-config.c
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#include "suse-config.h"

//FUNCIONES DEL CONFIG

int tamanio_vector(char** array){
	char* a = array[0];
	int i = 0;
	while(a!=NULL){
		i++;
		a = array[i];
	}
	return i;
}

int* pasar_a_vector_de_int(char** array){
	int longitud = tamanio_vector(array);
	int* vector = (int*)malloc(longitud*sizeof(int));

	for(int i=0; i<longitud; i++){
		vector[i] = (atoi(array[i]));
		free(array[i]);
	}
	free(array);
	return vector;
}

int* valores_iniciales_semaforos(){
	return pasar_a_vector_de_int(config_get_array_value(CONFIG, "SEM_INIT"));
}

int* valores_maximos_semaforos(){
	return pasar_a_vector_de_int(config_get_array_value(CONFIG, "SEM_MAX"));
}

char** ids_semaforos(){
	return config_get_array_value(CONFIG, "SEM_IDS");
}

int timerLog(){
	return config_get_int_value(CONFIG, "METRICS_TIMER");
}
int  puerto_listen(){
	return config_get_int_value(CONFIG, "LISTEN_PORT");
}

int grado_de_multiprogramacion_maximo(){
	return config_get_int_value(CONFIG, "MAX_MULTIPROG");
}

double alpha_sjf(){
	return config_get_double_value(CONFIG, "ALPHA_SJF");
}
