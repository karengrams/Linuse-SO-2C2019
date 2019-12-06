/*
 * muse-main.c
 *
 *  Created on: 4 dic. 2019
 *      Author: utnso
 */
#include "muse-main.h"
void atender_cliente_select(fd_set* master, int socketCli){
	t_paquete* paquete_respuesta = NULL;
	int cod_error;
	int id_cliente, cantidad_de_bytes, flags;
	void* buffer = NULL;
	void *path;
	t_proceso* cliente_a_atender = NULL;
	uint32_t direccion_pedida, direccion;
	t_list* paqueteRecibido = NULL;
	char* ipCli = (char*)malloc(sizeof(char)*20);
	ipCliente(socketCli, ipCli);

	int cod_op = recibir_operacion(socketCli);
	printf("Codigo operacion %d\n", cod_op);

	if(cod_op!=-1){
		paqueteRecibido = recibir_paquete(socketCli);
		cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
		switch(cod_op){
		case MUSE_INIT:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			cod_error = museinit(cliente_a_atender, ipCli, id_cliente);
			send(socketCli, &cod_error, sizeof(int), 0);
		break;
		case MUSE_CLOSE:
			mostrar_frames_ocupados();
			mostrar_tabla_de_segmentos(cliente_a_atender->tablaDeSegmentos);
			museclose(cliente_a_atender);
			printf("Luego de liberar al proceso...\n");
			mostrar_frames_ocupados();
		break;
		case MUSE_ALLOC:
			direccion = musealloc(cliente_a_atender, *((int*)list_get(paqueteRecibido,1)));
			send(socketCli, &direccion, sizeof(uint32_t), 0);
		break;
		case MUSE_FREE:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
			musefree(cliente_a_atender,direccion_pedida);
		break;
		case MUSE_GET: // TODO: ver caso de que se pase del limite
			cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
			buffer = museget(cliente_a_atender, paqueteRecibido);
			if (buffer == NULL){
				cod_error = -1;
				send(socketCli, &cod_error, sizeof(int), 0);
			} else {
				paquete_respuesta = crear_paquete(10);
				agregar_a_paquete(paquete_respuesta, buffer, cantidad_de_bytes);
				enviar_paquete(paquete_respuesta, socketCli);
				eliminar_paquete(paquete_respuesta);
			}
		break;
		case MUSE_CPY:
				cod_error = musecpy(cliente_a_atender, paqueteRecibido);
				send(socketCli, &cod_error, sizeof(int), 0);
		break;
		case MUSE_MAP:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			path = string_duplicate((char*)list_get(paqueteRecibido,1));
			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 2)); //este seria el length a mappear
			flags = *((int*)list_get(paqueteRecibido, 3));
			direccion = musemap(cliente_a_atender,path,cantidad_de_bytes,flags);
			send(socketCli, &direccion, sizeof(uint32_t), 0);
		break;
		case MUSE_SYNC:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 1)); //cantidad de bytes a guardar en el archivo
			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 2)); //direccion a partir de la cual hacer el sync
			musesync(cliente_a_atender,direccion_pedida,(size_t)cantidad_de_bytes);

			send(socketCli, &cod_error, sizeof(int), 0);
		break;
		case MUSE_UNMAP:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
			museunmap(cliente_a_atender,direccion_pedida);
			send(socketCli, &cod_error, sizeof(int), 0);
		break;
		}
	}
	free(ipCli);
	free(buffer);
		//free(paqueteRecibido); //TODO:fijarse como eliminar la lista de las commons

}

void atender_cliente(void *element){
	t_paquete* paquete_respuesta = NULL;
	int cod_error;
	int id_cliente, cantidad_de_bytes, flags;
	void* buffer = NULL;
	t_proceso* cliente_a_atender = NULL;
	uint32_t direccion_pedida, direccion;
	t_list* paqueteRecibido = NULL;
	char* ipCli = (char*)malloc(sizeof(char)*20);
	int socketCli = *((int*)element);
	ipCliente(socketCli, ipCli);
	while(1){

    	int cod_op = recibir_operacion(socketCli);
    	if(cod_op>=10 && cod_op<=18){
        	printf("\n\nSe ha solicitado la operacion nro. %d por el socket %d\n\n",cod_op,socketCli);
    		paqueteRecibido = recibir_paquete(socketCli);
    		cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    		switch(cod_op){
    		case MUSE_INIT:
    			mostrar_frames_ocupados();
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			cod_error = museinit(cliente_a_atender, ipCli, id_cliente);
    			send(socketCli, &cod_error, sizeof(int), 0);
    		break;
    		case MUSE_CLOSE:
    			printf("\nAntes de liberar al proceso...\n");
    			mostrar_tabla_de_segmentos(cliente_a_atender->tablaDeSegmentos);
    			museclose(cliente_a_atender);
    			printf("\nLuego de liberar al proceso...\n");
    			mostrar_frames_ocupados();
    		break;
    		case MUSE_ALLOC:
    			direccion = musealloc(cliente_a_atender, *((int*)list_get(paqueteRecibido,1)));
    			send(socketCli, &direccion, sizeof(uint32_t), 0);
    		break;
    		case MUSE_FREE:
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
    			musefree(cliente_a_atender,direccion_pedida);
    		break;
    		case MUSE_GET: // TODO: ver caso de que se pase del limite
    			cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
    			buffer = museget(cliente_a_atender, paqueteRecibido);
    			if (buffer == NULL){
    				cod_error = -1;
    				send(socketCli, &cod_error, sizeof(int), 0);
    			} else {
    				paquete_respuesta = crear_paquete(10);
    				agregar_a_paquete(paquete_respuesta, buffer, cantidad_de_bytes);
    				enviar_paquete(paquete_respuesta, socketCli);
    				eliminar_paquete(paquete_respuesta);
    			}
    		break;
    		case MUSE_CPY:
    				cod_error = musecpy(cliente_a_atender, paqueteRecibido);
    				send(socketCli, &cod_error, sizeof(int), 0);
    		break;
    		case MUSE_MAP:
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			char *path = string_duplicate((char*)list_get(paqueteRecibido,1));
    			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 2)); //este seria el length a mappear
    			flags = *((int*)list_get(paqueteRecibido, 3));
    			direccion = musemap(cliente_a_atender,path,cantidad_de_bytes,flags);
    			send(socketCli, &direccion, sizeof(uint32_t), 0);
    		break;
    		case MUSE_SYNC:
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 1)); //cantidad de bytes a guardar en el archivo
    			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 2)); //direccion a partir de la cual hacer el sync
    			musesync(cliente_a_atender,direccion_pedida,(size_t)cantidad_de_bytes);

    			send(socketCli, &cod_error, sizeof(int), 0);
    		break;
    		case MUSE_UNMAP:
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
    			museunmap(cliente_a_atender,direccion_pedida);
    			send(socketCli, &cod_error, sizeof(int), 0);
    		break;
    		}
    	}

	}
	free(ipCli);
	free(buffer);
		//free(paqueteRecibido); //TODO:fijarse como eliminar la lista de las commons

}

int main(void) {
	inicializar_recursos_de_memoria();

	signal(SIGINT,liberacion_de_recursos);


	int server_socket,client_socket;
	pthread_t hilo_de_atencion;

	server_socket=iniciar_socket("127.0.0.1",config_get_string_value(config,"LISTEN_PORT"));

	while(true){
		printf("Waiting for connections...\n");

		client_socket=esperar_cliente(server_socket);
		printf("Connected!\n");
		pthread_create(&hilo_de_atencion, NULL, &atender_cliente, &client_socket);

	}


// Comunicacion con select
//		Arranca a atender clientes
//		fd_set master;
//		fd_set read_fds;
//		int fdmax;
//		FD_ZERO(&master);
//		FD_ZERO(&read_fds);
//		int socketEs = iniciar_socket("127.0.0.1",config_get_string_value(config, "LISTEN_PORT"));
//
//		FD_SET(socketEs, &master);
//		fdmax = socketEs;
//
//		signal(SIGINT,liberacion_de_recursos);
//
//
//		while (1) {
//
//			read_fds = master;
//			select(fdmax + 1, &read_fds, NULL, NULL, NULL); // @suppress("Symbol is not resolved")
//
//			for (int i = 0; i <= fdmax; i++) {
//				if (FD_ISSET(i, &read_fds)) {
//					if (i == socketEs) {
//						admitir_nuevo_cliente(&master, &fdmax, i);
//					} else {
//						atender_cliente_select(&master, i);
//					}
//				}
//			}
//		}
	return 0;
}
