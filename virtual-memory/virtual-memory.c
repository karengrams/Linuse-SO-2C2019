#include "virtual-memory.h"

void* crear_archivo_swap(int SWAP_SIZE){
	void* map;
	int fd = open("swap.data", O_RDWR | O_CREAT, S_IRUSR);
	llenar_archivo(fd, SWAP_SIZE);
	map = mmap(NULL, SWAP_SIZE, PROT_WRITE, MAP_PRIVATE, fd, 0);
	return map;

}

void llenar_archivo(int fd, int tamanio){
	void* buffer = malloc(tamanio);
	char a = '\0';
	for(int i=0; i<tamanio; i++){
		memcpy(buffer+i, &a, 1);
	}

	write(fd, buffer, tamanio);
}

int posicion_libre_en_swap(){
	int posicionLibre = -1;

	for(int i=(bitarray_get_max_bit(BIT_ARRAY_SWAP)-1); i>=0; i--){

		if(!bitarray_test_bit(BIT_ARRAY_SWAP, i)){
			posicionLibre = i;
		}
	}

	return posicionLibre;

}

void inicializar_bitmap_swap(int SWAP_SIZE, int TAM_PAG){
	int bytes;
	int cantidadDeMarcos = SWAP_SIZE/TAM_PAG;

	div_t aux = div(cantidadDeMarcos, 8);

	if (aux.rem == 0) {
		bytes = aux.quot;
	} else {
		bytes = aux.quot + 1;
	}
	char *punteroABits = (char*) malloc(bytes); //Arreglar ese +1 en el malloc del bitmap de frames
	BIT_ARRAY_SWAP = bitarray_create_with_mode(punteroABits, (size_t) bytes,LSB_FIRST);
}

void asignar_marco_en_swap(page* pag){
	int posicionLibre = posicion_libre_en_swap();
	bitarray_set_bit(BIT_ARRAY_SWAP, (off_t) posicionLibre);
	pag->frame = NULL;
	pag->bit_presencia = false;
	pag->nro_frame = posicionLibre;
	pag->bit_uso = false;
	pag->bit_modificado = false;
}

void swap_pages(page* victima, page* paginaPedida){
	//datos de la victima
	int nroFrame = victima->nro_frame;
	frame* frameVictima = ((frame*)list_get(FRAMES_TABLE, nroFrame));

	int posicionEnSwap = paginaPedida->nro_frame*TAM_PAG;

	void* frameAReemplazar = frameVictima->memoria;
	void* bufferAux = malloc(TAM_PAG);

	memcpy(bufferAux, VIRTUAL_MEMORY+posicionEnSwap, TAM_PAG); //Swap mappeado como variable global por ahora
	memcpy(VIRTUAL_MEMORY+posicionEnSwap, frameAReemplazar, TAM_PAG);
	memcpy(frameAReemplazar, bufferAux, TAM_PAG);

	victima->bit_presencia = false;
	victima->frame = NULL;
	victima->nro_frame = paginaPedida->nro_frame;

	paginaPedida->bit_presencia = true;
	paginaPedida->frame = frameVictima;
	paginaPedida->nro_frame= nroFrame;
	paginaPedida->bit_uso = true;
	paginaPedida->bit_modificado = false;

	free(bufferAux);
}
