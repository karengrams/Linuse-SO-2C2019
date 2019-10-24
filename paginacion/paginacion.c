
t_list* dividir_memoria(void* memoria, int tamanioPagina, int tamanioMemoria){

	t_list* listaDeMarcos = list_create();
	int desplazamiento = 0;
	void* aux;

	while(desplazamiento < tamanioMemoria){
		aux = memoria+desplazamiento;
		list_add(listaDeMarcos, aux);
		desplazamiento += tamanioPagina;
	}

	return listaDeMarcos;

}

t_bitarray* crear_bitmap(t_list* listaDeMarcos){
	int bytes;
	int cantidadDeMarcos = list_size(listaDeMarcos);
	div_t aux = div(cantidadDeMarcos, 8);

		if (aux.rem == 0){
			bytes = aux.quot;
		} else {
			bytes = aux.quot + 1;
		}
	char* punteroABits = (char*)malloc(bytes);

	return bitarray_create_with_mode(punteroABits, bytes, LSB_FIRST);
}

int paginas_necesarias(int valorPedido){
	div_t aux = div((valorPedido+10), tamanio_paginas()); //Creo que va 10 porque irian dos metadatas (una al principio y otra al final) not sure tho
				if (aux.rem == 0){
					return aux.quot;
				} else {
					return aux.quot + 1;
				}
}
