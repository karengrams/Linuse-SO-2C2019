#include "../paginacion/paginacion.h"

t_list* crear_lista_paginas(int cantidadDePaginas){
	t_list *lista = malloc(sizeof(t_list));
	page *pagina;

	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(lista, pagina);
	}
	return lista;
}

int paginas_necesarias(int valorPedido){
	div_t aux = div((valorPedido+10), tamanio_paginas()); //Creo que va 10 porque irian dos metadatas (una al principio y otra al final) not sure tho
				if (aux.rem == 0){
					return aux.quot;
				} else {
					return aux.quot + 1;
				}
}

page* crear_pagina(){
	page* pagina = malloc(sizeof(page));
	pagina->bit_presencia = 0;
	return pagina;
}

void dividir_tabla_de_pags(int cantidadDePaginas, t_list *tabla_de_pags){
	page* pagina;

	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(tabla_de_pags, pagina);
	}
}



