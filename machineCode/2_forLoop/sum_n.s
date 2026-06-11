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


  // push regs to save  /* see: https://en.wikipedia.org/wiki/X86_calling_conventions */
  push ecx
  push edx
  
  mov ecx, 0
  mov eax, 0
  mov edx, [ebp+8]
  
                     /* glavna funkcionalnost potprograma */
  cmp edx, 0
  jz ret
  beg:  cmp ecx, edx 
	jg ret
	add eax, ecx
	add ecx, 1
	jmp beg
  
                     /* povratna vrijednost je zadržana u eax*/

ret:  pop edx
  pop ecx
                     /* cdecl epilog: */
  pop ebp            /* vrati (pozivateljev) ebp; umjesto 'add esp,4; pop ebp' može biti 'leave'*/
  ret                /* povratak iz potprograma */  

  
  
  