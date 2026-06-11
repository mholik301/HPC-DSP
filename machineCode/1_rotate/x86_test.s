// ovo je komentar
// funkcionira samo ako je jedina stvar u liniji. Inace treba koristiti /**/ sintaksu

// 0x... za hex kontante, ne ...h

// oznaka sintakse:
.intel_syntax noprefix

// neka simbol potprogram_asm
// bude vidljiv izvana:
.global potprogram_asm

// labela potprograma:
potprogram_asm:
	                 /* cdecl prolog: */
  push  ebp          /* spremi (pozivateljev) ebp; [ebp+4] je povratna adresa */
  mov   ebp, esp     /* ubaci (nas) esp u ebp */

                     /* zauzmi 8 bajta za lokalne varijable: */
  sub   esp, 4       /* lokalne varijable su "ispod" ebp */
					 /* umjesto ove 3 dosta je 'enter 4, 0' */

  // push regs to save  /* see: https://en.wikipedia.org/wiki/X86_calling_conventions */
  push eax
  push ebp
  


                     /* glavna funkcionalnost potprograma */
					 
  mov eax, 42  		 /* eax <- 42 ; if push eax and pop eax get commented out, program returns 42 */
  mov ebx, 0x42 	 /* eax <- 0x42 */
  
  push dx            /* Push lower 16 bits (yyyy) of EDX (xxxxyyyy), onto the stack */
  rol edx, 16 		 /* Rotate EDX bits left by 16 (AAAABBBB -> BBBBAAAA) */
  mov edx, 0xffff  	 /* Write 0xffff to EDX (0000ffff) */
  rol edx, 16 		 /* Rotate EDX bits left by 16 (0000ffff -> ffff0000) */
  pop dx             /* Pop yyyy off the stack into the lower 16 bits of EDX (ffffyyyy) */
  

  mov dl, 0xdd   	 /* Write 0xdd to EDX (xxxxxxdd) */
  
                     /* povratna vrijednost je zadržana u eax*/


                     /* oslobodi lokalne varijable:*/
  add   esp, 4		 /* or  mov esp, ebp */
  pop ebx
  pop eax
                     /* cdecl epilog: */
  pop ebp            /* vrati (pozivateljev) ebp; umjesto 'add esp,4; pop ebp' može biti 'leave'*/
  ret                /* povratak iz potprograma */  

  
  
  