

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

t_pagina* crear_pagina(){
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->bit_presencia = 0;
	pagina->numero_frame = -1;
	return pagina;
}

t_list* crear_lista_paginas(int cantidadDePaginas){
	t_list* lista = malloc(sizeof(t_list));
	t_pagina* pagina;

	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(lista, pagina);
	}
	return lista;
}
