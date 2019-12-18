#include "suse-main.h"

//HILO DE ATENCION A CLIENTES, UN HILO POR CADA SOCKET CONECTADO

void atenderCliente(void* elemento){
	int socketCli = *((int*)elemento);

	sem_t semaforoLoco;
	sem_init(&semaforoLoco, 0, 0);
	t_cosa* cosa = iniciar_cosa(&semaforoLoco, socketCli);
	list_add(LISTA_SEMAFOROS, cosa);


	t_list* paqueteRecibido = NULL, *colaAAplicarSJF = NULL;
	t_cola_ready* nodoReady;
	t_execute* nodoEnEjecucion;
	t_thread* hiloAEjecutar, *nuevoThread;
	int tid, posicionEnArray, error, valorsem;
	char* nombreSemaforo;
	bool _mismo_fd(void* elem){
			t_cola_ready* elemento = (t_cola_ready*)elem;
			return elemento->socket_fd == socketCli;
		}

		void _destruir_paquete(void* elem){
			free(elem);
		}
	while(1){

		int cod_op = recibir_operacion(socketCli);

		switch(cod_op){

		case SUSE_INIT:

			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);
			nuevoThread = crear_thread(socketCli, tid);

			if(!tid){ //si el tid es 0 es el programa principal
				crear_entrada_en_cola_ready(socketCli);
				nodoEnEjecucion = crear_entrada_en_lista_execute(socketCli);
				nodoEnEjecucion->thread = nuevoThread;
				gettimeofday(&nuevoThread->tiempo_en_cola_actual,NULL);

			} else {
				sem_wait(&sem_new);
				list_add(colaNEW, nuevoThread);
				sem_post(&sem_new);
				}
			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_SCHEDULE:

			sem_wait(&sem_ready);
			colaAAplicarSJF = ((t_cola_ready*)list_find(colaREADY, &_mismo_fd))->lista_threads; //buscamos la lista ready de este proceso
			sem_post(&sem_ready);
			nodoEnEjecucion = (t_execute*)list_find(listaEXEC, &_mismo_fd); //buscamos el hilo en ejecucion de este proceso

			if((nodoEnEjecucion->thread != NULL) && (list_size(colaAAplicarSJF) == 0)){
				tid = nodoEnEjecucion->thread->tid;
			} else {
				sem_wait(&semaforoLoco);
				colaAAplicarSJF = ((t_cola_ready*)list_find(colaREADY, &_mismo_fd))->lista_threads;
				hiloAEjecutar = algoritmo_SJF(colaAAplicarSJF); //Que esta funcion haga el remove de la cola de ready y lo retorne
				error = swap_threads(nodoEnEjecucion, hiloAEjecutar, colaAAplicarSJF); //mueve el hiloEnEjecucion a ready y el hiloAEjecutar a execute

				if(error == 0)
					sem_post(&semaforoLoco);

				tid = hiloAEjecutar->tid;
			}

			printf("Se elije al hilo %d para ejecutar \n", tid);
			send(socketCli, &tid, sizeof(int), 0); //mandamos el tid del hilo que pusimos a ejecutar
			break;

		case SUSE_JOIN:

			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);

			if(!tid_ya_esta_en_exit(tid, socketCli)){ //puede pasar que el thread al cual le hacen join ya termino
				buscar_y_pasarlo_a_blocked(socketCli, tid, JOIN);//Agarra el ult que esta en la cola exe de el socket y lo pasa a blocked por join con ese tid
			}

			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_SIGNAL:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)list_get(paqueteRecibido, 0));
			nombreSemaforo = list_get(paqueteRecibido,1);

			posicionEnArray = posicion_en_vector(nombreSemaforo, SEM_IDS);

			sem_wait(&semaforos_suse);
			error = hacer_signal(posicionEnArray);
			sem_post(&semaforos_suse);

			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_WAIT:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)list_get(paqueteRecibido, 0));
			nombreSemaforo = list_get(paqueteRecibido,1);
			posicionEnArray = posicion_en_vector(nombreSemaforo, SEM_IDS);

			sem_wait(&semaforos_suse);
			error = hacer_wait(posicionEnArray, socketCli);
			sem_post(&semaforos_suse);

			send(socketCli, &error, sizeof(int), 0);

			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_CLOSE:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);
			sem_wait(&sem_execute);
			buscar_y_pasarlo_a_exit(socketCli); //Agarra el ult que esta en la cola exe de el socket y lo pasa a exit
			sem_post(&sem_execute);
			buscar_hilos_blockeados_por_este(tid, socketCli);
			escribir_logs(tid, socketCli);

			if(!tid){
				list_destroy_and_destroy_elements(paqueteRecibido, &_destruir_paquete);
				close(socketCli);
				pthread_exit(NULL);
				return;
			} //Fin del hilo



			list_destroy_and_destroy_elements(paqueteRecibido, &_destruir_paquete);
			send(socketCli, &tid, sizeof(int), 0);
			break;
		}

	}
}

int main(){
	inicializar_recursos_de_planificador();

	//CREAMOS HILOS DE PLANIF Y ATENCION
	pthread_t hiloAtencion, hiloPlanif;
	int cliente;

	signal(SIGINT, &liberar_recursos); //Cuando generemos esta signal se liberan los recursos y termina el proceso
	signal(SIGALRM, &escribirLog);

	//ALARMA PARA LOGS
	struct itimerval intervalo;
	struct timeval tiempoInicial;

	tiempoInicial.tv_sec = timerLog();
	tiempoInicial.tv_usec = 0;

	intervalo.it_value = tiempoInicial;
	intervalo.it_interval = tiempoInicial;
	setitimer(ITIMER_REAL, &intervalo, NULL);

	SOCKET_ESCUCHA = iniciar_servidor("127.0.0.1", puerto_listen());

	pthread_create(&hiloPlanif, NULL, &planificador_largo_plazo, NULL);


	while(FUNCIONAR){
		cliente = esperar_cliente(SOCKET_ESCUCHA);

		if(cliente!=-1){
			pthread_create(&hiloAtencion, NULL, &atenderCliente, &cliente);
			pthread_detach(hiloAtencion);
		}
	}

	printf("\nADIOS\n");
	return EXIT_SUCCESS;
}
