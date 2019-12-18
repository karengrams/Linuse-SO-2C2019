#include <stdio.h>
#include <stdlib.h>
#include <hilolay/hilolay.h>
#include <unistd.h>

#define CANT_ITERACIONES 10000

void *tocar_solo()
{
	int i;

	printf("\nCAMPO: Golpeandose para entrar...\n");
	hilolay_yield();
	printf("\nCAMPO: Luchando para llegar al frente...\n");
	hilolay_yield();
	for(i=0;i<CANT_ITERACIONES;i++)
	{
		int accion = rand() % 5;
		if(!accion)
		{
			printf("CAMPO: POGO!!!!\n");
			hilolay_yield();
		}
		else
		{
			printf("CAMPO: Singing Along\n");
			hilolay_yield();
		}
	}
	return 0;
}

void *preparar_solo()
{
	int i;

	printf("\nPALCO: Estacionando el auto...\n");
	hilolay_yield();
	printf("\nPALCO: Haciendo fila para entrar...\n");
	hilolay_yield();
	for(i=0;i<CANT_ITERACIONES/100;i++)
	{
		int accion = rand() % 4;
		if(accion)
		{
			printf("PALCO: Aplaudiendo\n");
			hilolay_yield();
		}
		else
		{
			printf("PALCO: Singing Along\n");
			hilolay_yield();
		}
		usleep(100000);
	}
	return 0;
}

int main(void)
{
	struct hilolay_t palco;
	struct hilolay_t campo[3];

	hilolay_init();

	hilolay_create(&palco, NULL, &preparar_solo, NULL);
	hilolay_create(&campo[0], NULL, &tocar_solo, NULL);
	hilolay_create(&campo[1], NULL, &tocar_solo, NULL);
	hilolay_create(&campo[2], NULL, &tocar_solo, NULL);

	hilolay_join(&campo[0]);
	hilolay_join(&campo[1]);
	hilolay_join(&campo[2]);
	hilolay_join(&palco);


return hilolay_return(0);
}
