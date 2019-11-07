#include "paginacion.h"

page* crear_pagina(){
	page* pagina = malloc(sizeof(page));
	pagina->nro_pag=483204;
	pagina->bit_presencia = 0;
	pagina->frame=NULL;
	return pagina;
}

t_list* crear_tabla_de_paginas(int cantidadDePaginas){
	t_list *lista = list_create();
	page *pagina;

	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(lista, pagina);
	}
	return lista;
}

void agregar_paginas(t_list* tabla_de_paginas,int cantidadDePaginas){
	page *pagina;
	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(tabla_de_paginas, pagina);
	}
}

int paginas_necesarias(int valorPedido){
	div_t aux = div(valorPedido, TAM_PAG);
	if (aux.rem == 0){
		return aux.quot;
	}else{
		return aux.quot + 1;
	}
}
