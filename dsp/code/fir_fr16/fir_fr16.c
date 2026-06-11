/*****************************************************************************
* fir_fr16.c
*****************************************************************************/
#include <fract.h>
#include <cycle_count.h>
#include <stdio.h>
#include <ccblkfn.h>
#include <filter.h>
#include "signal_fract16.h"
#include "coeffs_fract16.h"

#define L 21
#define Nsamp 10

fract16 delay[L];
int idx_delay = 0;
fract16 izlbuf[Nsamp];
short idx_izlbuf = 0;

void obrada_fir_fr16(void)
{
	fir_state_fr16 filter_state;
	
	// initialize the filter_state struc
	fir_init(filter_state, coeffs, delay, L, 0);
	
	// run the operation
	fir_fr16(signal, izlbuf, Nsamp, &filter_state);
}

int main()
{
	int i,j;
	cycle_t start_count;
	cycle_t final_count;
	
	START_CYCLE_COUNT(start_count);
	obrada_fir_fr16();
	STOP_CYCLE_COUNT(final_count,start_count);
	printf("Number of cycles: %d\n",final_count);
	
	for(i=0;i<Nsamp;++i)
	{
		printf("%d\n", izlbuf[i]);
	}
	return 0;
}
