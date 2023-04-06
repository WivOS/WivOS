section .data

section .text

%macro ISR_NO_ERR_CODE 1
global isr%1
isr%1:
	push 0
	push %1
	jmp service_interrupt_normal
%endmacro

%macro ISR_ERR_CODE 1
global isr%1
isr%1:
	push %1
	jmp service_interrupt_normal
%endmacro

%macro ISR_NO_ERR_CODE_NO_SEGMENT_CHANGE 1
global isr%1
isr%1:
	push 0
	push %1
	jmp service_interrupt_normal
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
ISR_NO_ERR_CODE 32

%macro ISR_FILL 0
	%assign i 33
	%rep 256 - 33
		%if i = 64
			ISR_NO_ERR_CODE_NO_SEGMENT_CHANGE i
		%else
			ISR_NO_ERR_CODE i
		%endif
		%assign i i+1
	%endrep
%endmacro

%macro pusham 0
	cld
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
%endmacro

%macro popam 0
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
%endmacro

ISR_FILL

extern dispatch_interrupt

align 16
service_interrupt_normal:
    pusham

    mov rdi, rsp
	call dispatch_interrupt

    popam

    add rsp, 16
	iretq

align 16
service_interrupt_no_segment_change:
    pusham

    mov rdi, rsp
	call dispatch_interrupt

    popam

    add rsp, 16
	iretq
