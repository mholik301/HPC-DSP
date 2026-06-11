/*****************************************************************************
* fir_fract16.c
*****************************************************************************/
#include <fract.h>
#include <cycle_count.h>
#include <stdio.h>
#include <ccblkfn.h>
#include "signal_fract16.h"
#include "coeffs_fract16.h"

#define implB 1

#define L 21 /* broj uzoraka impulsnog odziva */
#define Nsamp 10

fract16 delay[L];

#ifndef implB
short idx_delay = 0;
#else
int idx_delay = 0;
#endif

fract16 izlbuf[Nsamp];
short idx_izlbuf = 0;

void obrada_fract16(fract16 sample)
{
	int i;
	fract32 acc = 0;
	fract32 prod;
	
	// zapamti najnoviji uzorak
	delay[idx_delay] = sample;
	
	// cirkularno povecaj indeks delay niza (circindex)
	idx_delay = circindex(idx_delay, 1, L);
	
	// kao i u prethodnim zadacima implementirajte mnozenje i akumulaciju
	// ovaj put koristeci fract16 i fract32 tipove podataka.
	// da bi to napravili, koristite naredbe mult_fr1x32, add_fr1x32 i
	// round_fr1x32
	for (i = 0; i < L; i++)
	{
		acc += round_fr1x32(add_fr1x32(acc, mult_fr1x32(delay[idx_delay], coeffs[i])));
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
		obrada_fract16(signal[i]);
		STOP_CYCLE_COUNT(final_count,start_count);
		printf("Number of cycles: %d\n",final_count);
		printf("%d\n", izlbuf[i]);
	}
	return 0;
}
