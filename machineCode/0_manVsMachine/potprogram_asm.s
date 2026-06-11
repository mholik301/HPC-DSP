// ovo je komentar

// oznaka sintakse:
.intel_syntax noprefix

// neka simbol potprogram_asm
// bude vidljiv izvana:
.global potprogram_asm

// labela potprograma:
potprogram_asm:
	                 /* cdecl prolog: */
  push  ebp          /* spremi ebp; [ebp+4] je povratna adresa */
  mov   ebp, esp     /* ubaci esp u ebp */

                     /* zauzmi 4 bajta za lokalne varijable: */
  sub   esp, 4       /* lokalne varijable su "ispod" ebp */
					 /* umjesto ove 3 dosta je 'enter 4, 0' */

  // push regs to save  /* see: https://en.wikipedia.org/wiki/X86_calling_conventions */
  // not neccessary here
  
  


                     /* glavna funkcionalnost potprograma */
  mov   eax, [ebp+12] /* b */
  add   eax, [ebp+8]  /* a */
  imul  eax, [ebp+16] /* c */
                     /* povratna vrijednost je zadržana u eax*/


                     /* oslobodi lokalne varijable:*/
  add   esp, 4

                     /* cdecl epilog: */
  pop ebp            /* umjesto 'add esp,4; pop ebp' može biti 'leave'*/
  ret                /* povratak iz potprograma */  
