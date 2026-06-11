/*****************************************************************************
* fir_integer.c
*****************************************************************************/
#include <cycle_count.h>
#include <stdio.h>
#include <ccblkfn.h>
#include "signal_integer.h"
#include "coeffs_integer.h"

#define L 21 /* broj uzoraka impulsnog odziva */
#define Nsamp 10

short delay[L];
short idx_delay = 0;
short izlbuf[Nsamp];
short idx_izlbuf = 0;

void obrada_integer(short sample)
{
	int i;
	int acc = 0;
	
	// zapamti najnoviji uzorak
	delay[idx_delay] = sample;
	
	// cirkularno povecaj indeks delay niza (circindex)
	idx_delay = circindex(idx_delay, 1, L);
	
	// u petlji akumuliraj umnoske stanja i koeficijenata, uz cirkularno
	// inkrementiranje indeksa stanja pomocu circindex
	// voditi racuna o specificnostima fixed point aritmetike, odnosno
	// posmaku u lijevo za jedan nakon mnozenja da bi rezultat bio tocan
	// u 32 bitnoj frakcionalnoj aritmetici (jeste li proucili pripremu?)
	// zadrzi samo gornjih 16 bitova akumulatora (za sad ne brinemo oko
	// overflowa) i posmakni na najmanje znacajna dva bajta (>>16)
	for (i = 0; i < L; i++)
	{
		acc += ((int)((int)delay[idx_delay] * (int)coeffs[i])<<1)>>16; // pomnozi, ukloni dupli bit predznaka, zaokruzi i zadrzi gornjih 16 bitova
		idx_delay = circindex(idx_delay, 1, L);
	}
	
	izlbuf[idx_izlbuf++] = acc;
}

int main()
{
	int i;
	cycle_t start_count;
	cycle_t final_count;
	for(i=0;i<Nsamp;++i)
	{
		START_CYCLE_COUNT(start_count);
		obrada_integer(signal[i]);
		STOP_CYCLE_COUNT(final_count,start_count);
		printf("Number of cycles: %d\n",final_count);
		printf("%d\n", izlbuf[i]);	//za provjeru ispis/2e15
	}
	return 0;
}
