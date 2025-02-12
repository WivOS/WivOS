section .data

%define smp_trampoline_size  smp_trampoline_end - smp_trampoline
smp_trampoline:              incbin "build/RELEASE/kernel/boot/trampoline.bin"
smp_trampoline_end:

section .text

extern gdt_setup_tss
extern syscall_entry

global smp_init_trampoline
smp_init_trampoline:
    mov byte [0x510], 0
    mov qword [0x520], rdi
    mov qword [0x540], rsi
    mov qword [0x550], rdx
    mov qword [0x560], rcx
    mov qword [0x590], syscall_entry
    sgdt [0x570]
    sidt [0x580]

    mov rsi, smp_trampoline
    mov rdi, 0x1000
    mov rcx, smp_trampoline_size
    rep movsb

    mov rdi, r8
    call gdt_setup_tss

    mov rax, 0x1
    ret

extern SecondSchedulerLock
extern SchedulerLock
extern ReSchedulerLock

global scheduler_switch_task
scheduler_switch_task:
    test rsi, rsi
    jz .do_not_update_page_table
    mov cr3, rsi

.do_not_update_page_table:
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

    ;                     rax  err+int_no  rip+cs+rflags
    mov rax, qword [rsp +  8  +    16    +      32      ] ; -> ss
    mov ds, ax
    mov es, ax

	pop rax

    add rsp, 16 ; Skip int_no and err

    lock inc dword [SecondSchedulerLock]
    lock inc dword [SchedulerLock]
    lock inc dword [ReSchedulerLock]

    iretq

extern scheduler_schedule_smp
extern scheduler_schedule
global force_reschedule
force_reschedule:
    cli

    mov rax, rsp
    push 0x10            ; ss
    push rax             ; rsp
    push 0x202           ; rflags
    push 0x08            ; cs
    mov rax, .done
    push rax             ; rip

    push 0x0 ; Error
    push 0x0 ; Int no

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

    lock inc dword [SchedulerLock]
    ;lock btsw [SchedulerLock]

    mov rdi, rsp
.retry:
    xor rbp, rbp
    call scheduler_schedule_smp
    ;xor rbp, rbp
    ;call scheduler_schedule
    jmp .retry

.done:
    ret
