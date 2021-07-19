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

%macro ISR_NO_ERR_CODE2 1
global isr%1
isr%1:
	jmp service_interrupt2
%endmacro

%macro ISR_NO_ERR_CODE3 1
global isr%1
isr%1:
	jmp service_interrupt3
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
ISR_NO_ERR_CODE3 32

%macro ISR_FILL 0
	%assign i 33
	%rep 65 - 33
		ISR_NO_ERR_CODE i
		%assign i i+1
	%endrep
%endmacro

ISR_NO_ERR_CODE2 65

%macro ISR_FILL2 0
	%assign i 66
	%rep 256 - 66
		ISR_NO_ERR_CODE i
		%assign i i+1
	%endrep
%endmacro

ISR_FILL
ISR_FILL2

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

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	;mov ss, ax
	;mov gs, ax

	mov rdi, rsp
	call dispatch_interrupt

	pop rax

	mov ds, ax
	mov es, ax
	;mov ss, ax
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

align 16
global service_interrupt2
service_interrupt2:
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

	mov rdi, rsp
    xor rbp, rbp

    extern ipi_resched
	call ipi_resched

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

	iretq

align 16
global service_interrupt3
service_interrupt3:
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

	mov rdi, rsp
    xor rbp, rbp

    extern pit_handler
	call pit_handler

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

	iretq

invalid_syscall:
    mov rax, -1
    ret

syscall_count equ ((syscall_table.end - syscall_table) / 8)
syscall_table:
    extern debug_syscall
    dq debug_syscall ; 0x0
    extern open_syscall
    dq open_syscall ; 0x1
    extern read_syscall
    dq read_syscall ; 0x2
    extern write_syscall
    dq write_syscall ; 0x3
    extern close_syscall
    dq close_syscall ; 0x4
    extern ioctl_syscall
    dq ioctl_syscall ; 0x5
    extern alloc_at_syscall
    dq alloc_at_syscall ; 0x6
    extern fork_syscall
    dq fork_syscall ; 0x7
    extern lseek_syscall
    dq lseek_syscall ; 0x8
    extern isatty_syscall
    dq isatty_syscall ; 0x9
    dq invalid_syscall
.end:

global syscall_entry
syscall_entry:
    mov qword [gs:0024], rsp
    mov rsp, qword [gs:0016]

    sti

    push 0x1b            ; ss
    push qword [gs:0024] ; rsp
    push r11             ; rflags
    push 0x23            ; cs
    push rcx             ; rip

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

    cmp rax, syscall_count
    jae .err

    mov rdi, rsp
    xor rbp, rbp
    call [syscall_table + rax * 8]

.out:
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

    cli

    mov rsp, qword [gs:0024]

    o64 sysret

.err:
    mov rax, -1
    jmp .out

global smp_check_ap_flag
smp_check_ap_flag:
    xor rax, rax
    mov al, byte [0x510]
    ret

global smp_init_trampoline
extern gdt_load_tss
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

    mov rdi, r8
    call gdt_load_tss

    mov rax, 0x1
    ret

global smp_init_cpu0_local
smp_init_cpu0_local:
    mov ax, 0x1b
    mov fs, ax
    mov gs, ax
    mov rcx, 0xc0000101
    mov eax, edi
    shr rdi, 32
    mov edx, edi
    wrmsr

    mov rdi, rsi
    call gdt_load_tss

    mov ax, 0x28
    ltr ax

    ret

global task_return_context
task_return_context:
    test rsi, rsi
    jz .dont_load_cr3
    mov cr3, rsi
    
.dont_load_cr3:
    mov rsp, rdi

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

    mov rax, qword [rsp+32+8]
    mov ds, ax
    mov es, ax

    pop rax

    sti

    iretq

global force_resched
force_resched:
    cli

    mov rax, rsp

    push 0x10
    push rax
    push 0x202
    push 0x08
    mov rax, .done
    push rax
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

    extern schedulerLock
    lock inc qword [schedulerLock]

    mov rdi, rsp
.retry:
    xor rbp, rbp
    extern schedule
    call schedule
    jmp .retry

.done:
    ret

;Todo: Remove this when not needed.
align 16
global test_function
test_function:
    ; Test function to test if executing from userland space works
    int 0x3
    int 0x3
    jmp $
global end_test_function
end_test_function: