section .data

%define smp_trampoline_size  smp_trampoline_end - smp_trampoline
smp_trampoline:              incbin "build/RELEASE/kernel/proc/trampoline.bin"
smp_trampoline_end:

section .text

%macro ISR_NO_ERR_CODE 1
global isr%1
isr%1:
	push 0
	push %1
	jmp service_interrupt
%endmacro

%macro ISR_ERR_CODE 1
global isr%1
isr%1:
	push %1
	jmp service_interrupt
%endmacro

ISR_NO_ERR_CODE 0
ISR_NO_ERR_CODE 1
ISR_NO_ERR_CODE 2
ISR_NO_ERR_CODE 3
ISR_NO_ERR_CODE 4
ISR_NO_ERR_CODE 5
ISR_NO_ERR_CODE 6
ISR_NO_ERR_CODE 7
ISR_ERR_CODE 8
ISR_NO_ERR_CODE 9
ISR_ERR_CODE 10
ISR_ERR_CODE 11
ISR_ERR_CODE 12
ISR_ERR_CODE 13
ISR_ERR_CODE 14
ISR_NO_ERR_CODE 15
ISR_NO_ERR_CODE 16
ISR_ERR_CODE 17
ISR_NO_ERR_CODE 18
ISR_NO_ERR_CODE 19
ISR_NO_ERR_CODE 20
ISR_NO_ERR_CODE 21
ISR_NO_ERR_CODE 22
ISR_NO_ERR_CODE 23
ISR_NO_ERR_CODE 24
ISR_NO_ERR_CODE 25
ISR_NO_ERR_CODE 26
ISR_NO_ERR_CODE 27
ISR_NO_ERR_CODE 28
ISR_NO_ERR_CODE 29
ISR_ERR_CODE 30
ISR_NO_ERR_CODE 31

%macro ISR_FILL 0
	%assign i 32
	%rep 256 - 32
		ISR_NO_ERR_CODE i
		%assign i i+1
	%endrep
%endmacro

ISR_FILL

extern dispatch_interrupt

service_interrupt:
    push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	xor rax, rax
	mov ax, ds
	push rax

	mov ax, 0x0
	mov ds, ax
	mov es, ax
	;mov fs, ax
	;mov gs, ax

	mov rdi, rsp
	call dispatch_interrupt

	pop rax

	mov ds, ax
	mov es, ax
	;mov fs, ax
	;mov gs, ax

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax

	add rsp, 16
	iretq

global smp_check_ap_flag
smp_check_ap_flag:
    xor rax, rax
    mov al, byte [0x510]
    ret

global smp_init_trampoline
smp_init_trampoline:
    mov byte [0x510], 0
    mov qword [0x520], rdi
    mov qword [0x540], rsi
    mov qword [0x550], rdx
    mov qword [0x560], rcx
    sgdt [0x570]
    sidt [0x580]

    mov rsi, smp_trampoline
    mov rdi, 0x1000
    mov rcx, smp_trampoline_size
    rep movsb

    mov rax, 0x1
    ret