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
%endmacro

global syscall_entry
align 16
syscall_entry:
	mov qword [gs:0024], rsp
    mov rsp, qword [gs:0016]

	sti

	push 0x1B            ; ss
    push qword [gs:0024] ; rsp
    push r11             ; rflags
    push 0x23            ; cs
    push rcx             ; rip

	push 0x0; err
	push 0x0; int_no

	pusham

	cmp rax, syscall_count
	jae .err

	mov rdi, rsp
	xor rbp, rbp
	call [syscall_table + rax * 8]

.out:
	popam

	cli

	mov rsp, qword [gs:0024]

    o64 sysret

.err:
	mov rdi, rsp
	xor rbp, rbp
	call syscall_module_or_invalid
    jmp .out

section .data

syscall_count equ ((syscall_table.end - syscall_table) / 8)
syscall_table:
    extern syscall_debug
	extern syscall_read
	extern syscall_write
	extern syscall_open
	extern syscall_close
	extern syscall_alloc
	extern syscall_seek
	extern syscall_tcb_set
	extern syscall_fork
	extern syscall_ioctl
	extern syscall_module_or_invalid
    dq syscall_debug 	; 0x0
	dq syscall_read		; 0x1 -> read
	dq syscall_write	; 0x2 -> write
	dq syscall_open		; 0x3 -> open
	dq syscall_close	; 0x4 -> close
	dq syscall_alloc	; 0x5 -> alloc
	dq syscall_seek		; 0x6 -> seek
	dq syscall_tcb_set	; 0x7 -> tcb_set
	dq syscall_fork		; 0x8 -> fork
	dq syscall_ioctl	; 0x9 -> ioctl
    dq syscall_module_or_invalid
.end: