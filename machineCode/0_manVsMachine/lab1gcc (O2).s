	.file	"lab1gcc.cpp"
	.intel_syntax noprefix
	.text
	.p2align 4,,15
	.def	___tcf_0;	.scl	3;	.type	32;	.endef
___tcf_0:
LFB1923:
	.cfi_startproc
	mov	ecx, OFFSET FLAT:__ZStL8__ioinit
	jmp	__ZNSt8ios_base4InitD1Ev
	.cfi_endproc
LFE1923:
	.p2align 4,,15
	.globl	__Z12potprogram_ciii
	.def	__Z12potprogram_ciii;	.scl	2;	.type	32;	.endef
__Z12potprogram_ciii:
LFB1489:
	.cfi_startproc
	mov	eax, DWORD PTR [esp+8]
	add	eax, DWORD PTR [esp+4]
	imul	eax, DWORD PTR [esp+12]
	ret
	.cfi_endproc
LFE1489:
	.def	___main;	.scl	2;	.type	32;	.endef
	.section .rdata,"dr"
LC0:
	.ascii "ASM: \0"
LC1:
	.ascii "C++: \0"
	.section	.text.startup,"x"
	.p2align 4,,15
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
LFB1490:
	.cfi_startproc
	lea	ecx, [esp+4]
	.cfi_def_cfa 1, 0
	and	esp, -16
	push	DWORD PTR [ecx-4]
	push	ebp
	.cfi_escape 0x10,0x5,0x2,0x75,0
	mov	ebp, esp
	push	ebx
	push	ecx
	.cfi_escape 0xf,0x3,0x75,0x78,0x6
	.cfi_escape 0x10,0x3,0x2,0x75,0x7c
	sub	esp, 16
	call	___main
	mov	DWORD PTR [esp+8], 6
	mov	DWORD PTR [esp+4], 5
	mov	DWORD PTR [esp], 3
	call	potprogram_asm
	mov	DWORD PTR [esp+8], 5
	mov	ebx, eax
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC0
	mov	DWORD PTR [esp], OFFSET FLAT:__ZSt4cout
	call	__ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i
	mov	ecx, OFFSET FLAT:__ZSt4cout
	mov	DWORD PTR [esp], ebx
	call	__ZNSolsEi
	sub	esp, 4
	mov	DWORD PTR [esp], eax
	call	__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_
	mov	DWORD PTR [esp+8], 5
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC1
	mov	DWORD PTR [esp], OFFSET FLAT:__ZSt4cout
	call	__ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i
	mov	ecx, OFFSET FLAT:__ZSt4cout
	mov	DWORD PTR [esp], 48
	call	__ZNSolsEi
	sub	esp, 4
	mov	DWORD PTR [esp], eax
	call	__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_
	lea	esp, [ebp-8]
	xor	eax, eax
	pop	ecx
	.cfi_restore 1
	.cfi_def_cfa 1, 0
	pop	ebx
	.cfi_restore 3
	pop	ebp
	.cfi_restore 5
	lea	esp, [ecx-4]
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE1490:
	.p2align 4,,15
	.def	__GLOBAL__sub_I__Z12potprogram_ciii;	.scl	3;	.type	32;	.endef
__GLOBAL__sub_I__Z12potprogram_ciii:
LFB1924:
	.cfi_startproc
	sub	esp, 28
	.cfi_def_cfa_offset 32
	mov	ecx, OFFSET FLAT:__ZStL8__ioinit
	call	__ZNSt8ios_base4InitC1Ev
	mov	DWORD PTR [esp], OFFSET FLAT:___tcf_0
	call	_atexit
	add	esp, 28
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
LFE1924:
	.section	.ctors,"w"
	.align 4
	.long	__GLOBAL__sub_I__Z12potprogram_ciii
.lcomm __ZStL8__ioinit,1,1
	.ident	"GCC: (MinGW.org GCC-6.3.0-1) 6.3.0"
	.def	__ZNSt8ios_base4InitD1Ev;	.scl	2;	.type	32;	.endef
	.def	__ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i;	.scl	2;	.type	32;	.endef
	.def	__ZNSolsEi;	.scl	2;	.type	32;	.endef
	.def	__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_;	.scl	2;	.type	32;	.endef
	.def	__ZNSt8ios_base4InitC1Ev;	.scl	2;	.type	32;	.endef
	.def	_atexit;	.scl	2;	.type	32;	.endef
