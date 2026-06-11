/*****************************************************************************
* fir_float.c
*****************************************************************************/
#include <cycle_count.h>
#include <stdio.h>
#include <ccblkfn.h>

//#define implB 1

#define Nsamp 10
#define L 21

#include "signal_float.h"
#include "coeffs_float.h"

float delay[L];			// uzorci
short idx_delay = 0;	// indeks "najstarijeg" uzorka
float izlbuf[Nsamp];	// spremnik izlaznih vrijednosti
short idx_izlbuf = 0;	// indeks izlaznih vrijednosti

void obrada_float(float sample)
{
	int i;
	float acc = 0;
	// zapamti najnoviji uzorak
	delay[idx_delay] = sample;
	
	// cirkularno povecaj indeks delay niza (vidi napomenu)
	#ifndef implB
	idx_delay = (idx_delay+1) % L;
	#else
	idx_delay = circindex(idx_delay, 1, L);	//idx, incr, len
	#endif
	
	// u petlji akumuliraj umnoske stanja i koeficijenata, uz cirkularno
	// inkrementiranje indeksa stanja (delay)
	for(i=0;i<L;i++)
	{
		acc += delay[idx_delay]*coeffs[i];
		#ifndef implB
		idx_delay = (idx_delay+1) % L;
		#else
		idx_delay = circindex(idx_delay, 1, L);	//idx, incr, len
		#endif
	}
	
	izlbuf[idx_izlbuf++] = acc;
}

int main()
{
	int i;
	cycle_t start_count;
	cycle_t final_count;
	int final = 0;
	for(i=0;i<Nsamp;++i)
	{
		START_CYCLE_COUNT(start_count);
		obrada_float(signal[i]);
		STOP_CYCLE_COUNT(final_count,start_count);
		PRINT_CYCLES("Number of cycles: ",final_count);
		final += final_count;
		printf("%f\n", izlbuf[i]);
	}
	
	printf("final=%d\n", final);

	return 0;
}