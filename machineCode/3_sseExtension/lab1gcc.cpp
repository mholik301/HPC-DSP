#include <iostream>

extern "C" int potprogram_asm_87(float const* A, float const* B, int count, float *R) asm("potprogram_asm_87");  
extern "C" int potprogram_asm_sse(float const* A, float const* B, int count, float *R) asm("potprogram_asm_sse");  


void __cdecl potprogram_c(float const* A, float const* B, int count, float *R){
	for (int i=0; i<count; i++){	//n+1 jer i krece od 0, a ne 1
		*R+=A[i];
		*R+=B[i];
	}
  }  

int main(){
	//note: radi jednostavnosti implementacije _sse programa, ulazna polja/count mora biti visekratnik broja 4
	
	int count=8;
	float A[count] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}; //sum=36
	float B[count] = {11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0}; //sum=116; total=152
	float R=0;
	
	potprogram_c(A, B, count, &R);
	std::cout <<"C++: " << R <<std::endl;
	
	R=0;
	potprogram_asm_87(A, B, count, &R);
	std::cout <<"ASM_87: " << R <<std::endl;
	
	R=0;
	potprogram_asm_sse(A, B, count, &R);
	std::cout <<"ASM_sse: " << R <<std::endl;
}
