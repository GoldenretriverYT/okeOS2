[extern handle_isr]
[extern handle_irq]
[global idt_stub_table]

%macro macroPusha 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro macroPopa 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro errisr 1
stub%+%1:
    push %1

    macroPusha
    mov rdi, rsp
    call handle_isr
    macroPopa

    add rsp, 16
    sti
    iretq
%endmacro

%macro noerrisr 1
stub%+%1:
    push 0
    push %1

    macroPusha
    mov rdi, rsp
    call handle_isr
    macroPopa

    add rsp, 16
    sti
    iretq
%endmacro

%macro irq 1
stub%+%1:
    push 0
    push %1
    xchg bx, bx ; Bochs magic breakpoint

    macroPusha
    mov rdi, rsp
    call handle_irq
    macroPopa

    add rsp, 16
    sti
    xchg bx, bx ; Bochs magic breakpoint
    iretq
%endmacro

; Exception ISRs
noerrisr 0
noerrisr 1
noerrisr 2
noerrisr 3
noerrisr 4
noerrisr 5
noerrisr 6
noerrisr 7
errisr 8
noerrisr 9
errisr 10
errisr 11
errisr 12
errisr 13
errisr 14
noerrisr 15
noerrisr 16
errisr 17
noerrisr 18
noerrisr 19
noerrisr 20
noerrisr 21
noerrisr 22
noerrisr 23
noerrisr 24
noerrisr 25
noerrisr 26
noerrisr 27
noerrisr 28
noerrisr 29
noerrisr 30
noerrisr 31

; IRQs
%assign irqnum 32
%rep 224
    irq irqnum
    %assign irqnum irqnum+1
%endrep

section .data
idt_stub_table:
    %assign stubidx 0
    %rep 256
        dq stub%+stubidx
        %assign stubidx stubidx+1
    %endrep