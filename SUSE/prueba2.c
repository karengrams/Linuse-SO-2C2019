#include <stdlib.h>
#include <stdio.h>
#include <hilolay/hilolay.h>

#define CANT_NOTAS 420

struct hilolay_sem_t *solo_hiper_mega_piola;
struct hilolay_sem_t *afinado;

void *tocar_solo(void* num)
{
	int cont = 0;

	for(int i = 0; i < CANT_NOTAS/4; i++)
	{
		hilolay_wait(afinado);
		hilolay_wait(solo_hiper_mega_piola);
		cont++;
		printf("%d: PARAPAPAM! Nota %d\n", num, cont);
		hilolay_signal(solo_hiper_mega_piola);
		hilolay_yield();
	}

	printf("\nPude tocar %d notas bien\n", cont);
	return 0;
}

int main(void)
{
	struct hilolay_t guitarrista[4];

	hilolay_init();

	solo_hiper_mega_piola = hilolay_sem_open("solo_hiper_mega_piola");
	afinado = hilolay_sem_open("afinado");

	hilolay_create(&guitarrista[0], NULL, &tocar_solo, (void*)0);
	hilolay_create(&guitarrista[1], NULL, &tocar_solo, (void*)1);
	hilolay_create(&guitarrista[2], NULL, &tocar_solo, (void*)2);
	hilolay_create(&guitarrista[3], NULL, &tocar_solo, (void*)3);

	hilolay_join(&guitarrista[0]);
	hilolay_join(&guitarrista[1]);
	hilolay_join(&guitarrista[2]);
	hilolay_join(&guitarrista[3]);

	hilolay_sem_close(solo_hiper_mega_piola);
	hilolay_sem_close(afinado);

	return hilolay_return(0);
}
