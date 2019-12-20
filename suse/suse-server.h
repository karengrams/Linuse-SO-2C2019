/*
 * suse-server.h
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#ifndef SUSE_SERVER_H_
#define SUSE_SERVER_H_

#include "suse-structures.h"
#include "suse-config.h"
#include "suse-main.h"

void crear_entrada_en_cola_ready(int );
t_execute* crear_entrada_en_lista_execute(int);
t_thread* crear_thread(int, int );
void agregar_a_ready(int , t_thread* );
void swap_threads(t_execute* , t_thread* );
long double rafaga_estimada(void* );
bool _menor_rafaga_estimada(void* , void* );
t_thread* algoritmo_SJF(t_list*);
void buscar_hilos_blockeados_por_este(int, int);
void buscar_y_pasarlo_a_exit(int );
t_blocked* crear_entrada_blocked(t_thread* , t_estado , int );
bool tid_ya_esta_en_exit(int , int );
void buscar_y_pasarlo_a_blocked(int fd,int , t_estado );
int posicion_en_vector(char* , char** );
int hacer_signal(int );
int hacer_wait(int , int );
t_cosa* iniciar_cosa(sem_t* , int );
void _liberar_exit(void* );
void _liberar_ready(void* );
void _liberar_execute(void* );
void _liberar_semaforos(void* );
void liberar_sem_ids();
void liberar_recursos(int sig);
void iniciar_semaforos();
void inicializar_recursos_de_planificador();

#endif /* SUSE_SERVER_H_ */
