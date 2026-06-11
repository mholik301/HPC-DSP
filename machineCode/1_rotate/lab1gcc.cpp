#include <iostream>

extern "C" int potprogram_asm() asm("potprogram_asm");  

int __cdecl potprogram_c(int a, int b, int c) {
    return (a + b) * c;
  }  

int main(){
	std::cout <<"ASM: " <<potprogram_asm() <<std::endl;
	std::cout <<"C++: " <<potprogram_c(3,5,6) <<std::endl;
} 