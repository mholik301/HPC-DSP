
// oznaka sintakse:
.intel_syntax noprefix

// neka simbol potprogram_asm_87
// bude vidljiv izvana:
.global potprogram_asm_87
.global potprogram_asm_sse


// labela potprograma:
potprogram_asm_87:
	                 /* cdecl prolog: */
  push  ebp          /* spremi (pozivateljev) ebp; [ebp+4] je povratna adresa */
  mov   ebp, esp     /* ubaci (nas) esp u ebp */


  // push regs to save  /* see: https://en.wikipedia.org/wiki/X86_calling_conventions */
  push edi
  
  mov edi, [ebp+20]	/* *R */
  mov edx, [ebp+16] /* count */
  mov ecx, 0		/* i */
  mov ebx, [ebp+12]	/* B */
  mov eax, [ebp+8]	/* A */

                     /* glavna funkcionalnost potprograma */
  cmp edx, 0
  jz ret_87
  fldz
  beg_87:  cmp ecx, edx 
	jge store_87
	
	fld DWORD PTR [eax+ecx*4]	/* load A[i] on float stack */
	faddp						/* stack=stack+A[i] */
	fld DWORD PTR [ebx+ecx*4]	/* load B[i] on float stack  */
	faddp						/* stack=stack+B[i] */
	
	add ecx, 1
	jmp beg_87
  
  store_87: fstp DWORD PTR [edi]


ret_87:  pop edi
                     /* cdecl epilog: */
  pop ebp            /* vrati (pozivateljev) ebp; umjesto 'add esp,4; pop ebp' može biti 'leave'*/
  ret                /* povratak iz potprograma */  

  
  
  
  
 
// labela potprograma:
potprogram_asm_sse:
	                 /* cdecl prolog: */
  push  ebp          /* spremi (pozivateljev) ebp; [ebp+4] je povratna adresa */
  mov   ebp, esp     /* ubaci (nas) esp u ebp */


  // push regs to save  /* see: https://en.wikipedia.org/wiki/X86_calling_conventions */
  push edi
  push esi
  
  mov edi, [ebp+20]	/* *R */
  mov edx, [ebp+16] /* count */
  mov ecx, 0		/* i */
  mov ebx, [ebp+12]	/* B */
  mov eax, [ebp+8]	/* A */

                     /* glavna funkcionalnost potprograma */
  cmp edx, 0
  jz ret_sse
  beg_sse:  cmp ecx, edx 	/*if i>count*/
	jge store_sse			/*then jump to store*/
	
	//mov esi, edx		/*esi=count*/
	//sub esi, ecx		/*esi=count-i*/
	//sub esi, 4		/*esi=count-i-4*/
	//cmp esi, 0		/*if there are more then 4 elements left*/
	//jge n				/*continue normal*/
	//xor xmm0, xmm0		/*else set all bits of xmm0 to 0*/
	//load only esi values into xmm somehow?
	
	n: movaps xmm0, [eax+ecx*4] /* load A[i, i+1, i+2, i+3] in xmm0 */
	addps xmm0, [ebx+ecx*4]	/* xmm0 += B[i, i+1, i+2, i+3] */
	addps xmm1, xmm0			/* sum=xmm1 += xmm0 */
	
	add ecx, 4 /*i+=4*/
	jmp beg_sse
  
  store_sse: haddps xmm1, xmm1 /*xmm1[31:0]=xmm1[63:32]+xmm1[31:0], xmm1[63:32]=xmm1[127:96]+xmm1[95:64]*/
	haddps xmm1, xmm1 /*xmm1[31:0]=xmm1[63:32]+xmm1[31:0]*/
	movss DWORD PTR [edi], xmm1


ret_sse: pop esi  
	pop edi
                     /* cdecl epilog: */
  pop ebp            /* vrati (pozivateljev) ebp; umjesto 'add esp,4; pop ebp' može biti 'leave'*/
  ret                /* povratak iz potprograma */  
 
