#include <iostream>

extern "C" int potprogram_asm(int n) asm("potprogram_asm");  

int __cdecl potprogram_c(int n) {
	int sum=0;
	for (int i=0; n+1>i; i){	//n+1 jer i krece od 0, a ne 1
		sum+=i;
		i++;
	}
    return sum;
  }  

int main(){
	std::cout <<"ASM: " <<potprogram_asm(3) <<std::endl;
	std::cout <<"C++: " <<potprogram_c(3) <<std::endl;
} 