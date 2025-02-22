/*
 * muse-main.c
 *
 *  Created on: 4 dic. 2019
 *      Author: utnso
 */
#include "muse-main.h"

void _eliminar_elementos_paquete(void*element){
   		free(element);
}

void* atender_cliente(void *element){
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
		if(cod_op == -1){
			close(socketCli);
			//send(socketCli, &tid, sizeof(int), 0);
			pthread_exit(NULL);
		}
    	if(cod_op>=10 && cod_op<=18 ){
    	paqueteRecibido = recibir_paquete(socketCli);
    		switch(cod_op){
    		case MUSE_INIT:
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			cod_error = museinit(cliente_a_atender, ipCli, id_cliente);
    			send(socketCli, &cod_error, sizeof(int), 0);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_CLOSE:
    	    	cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			museclose(cliente_a_atender);
    			close(socketCli);
				pthread_exit(NULL);
				free(ipCli);
				free(buffer);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    			break;
    		case MUSE_ALLOC:
    	    	cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			direccion = musealloc(cliente_a_atender, *((int*)list_get(paqueteRecibido,1)));
    			send(socketCli, &direccion, sizeof(uint32_t), 0);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_FREE:
    			cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
    			cod_error = musefree(cliente_a_atender,direccion_pedida);
    			send(socketCli, &cod_error, sizeof(int), 0);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_GET:
    			cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
    			buffer = museget(cliente_a_atender, paqueteRecibido);
    			if (buffer == NULL){
    				cod_error = -1;
    				send(socketCli, &cod_error, sizeof(int), 0);
        			close(socketCli);
    				pthread_exit(NULL);
    				free(ipCli);
    				free(buffer);
    			} else {
    				paquete_respuesta = crear_paquete(10);
    				agregar_a_paquete(paquete_respuesta, buffer, cantidad_de_bytes);
    				enviar_paquete(paquete_respuesta, socketCli);
    				eliminar_paquete(paquete_respuesta);
    			}
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_CPY:
    	    	cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			cod_error = musecpy(cliente_a_atender, paqueteRecibido);
    			send(socketCli, &cod_error, sizeof(int), 0);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_MAP:
    	    	cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			char *path = string_duplicate((char*)list_get(paqueteRecibido,1));
    			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 2)); //este seria el length a mappear
    			flags = *((int*)list_get(paqueteRecibido, 3));
    			direccion = musemap(cliente_a_atender,path,cantidad_de_bytes,flags);
    			send(socketCli, &direccion, sizeof(uint32_t), 0);

			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_SYNC:
    	    	cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 1)); //cantidad de bytes a guardar en el archivo
    			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 2)); //direccion a partir de la cual hacer el sync
    			musesync(cliente_a_atender,direccion_pedida,(size_t)cantidad_de_bytes);
    			send(socketCli, &cod_error, sizeof(int), 0);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		case MUSE_UNMAP:
    	    	cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
    			id_cliente = *((int*)list_get(paqueteRecibido, 0));
    			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
    			museunmap(cliente_a_atender,direccion_pedida);
    			send(socketCli, &cod_error, sizeof(int), 0);
			  	list_destroy_and_destroy_elements(paqueteRecibido,_eliminar_elementos_paquete);
    		break;
    		}
    	}

	}


}

int main(void) {
	inicializar_recursos_de_memoria();

	signal(SIGINT,liberacion_de_recursos);

	int server_socket,client_socket;
	pthread_t hilo_de_atencion;

	server_socket=iniciar_socket(config_get_int_value(config,"LISTEN_PORT"));
	printf("Connected!\nWaiting for connections...\n");
	while(true){
		client_socket=esperar_cliente(server_socket);
		if(client_socket!=-1){
			pthread_create(&hilo_de_atencion, NULL, &atender_cliente, &client_socket);
			pthread_detach(hilo_de_atencion);
		}

	}
	return 0;
}
