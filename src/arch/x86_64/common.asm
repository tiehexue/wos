%ifndef COMMON_ASM
%define COMMON_ASM

%macro restore_kernel_segments 0
    push rax
    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov es, eax
    mov gs, eax
    pop rax
%endmacro

%macro restore_user_segments 0
    pop rax
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
%endmacro

%macro save_context 0
    push rbp
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    sub rsp, 256
    movdqu [rsp], xmm15
    movdqu [rsp+16], xmm14
    movdqu [rsp+32], xmm13
    movdqu [rsp+48], xmm12
    movdqu [rsp+64], xmm11
    movdqu [rsp+80], xmm10
    movdqu [rsp+96], xmm9
    movdqu [rsp+112], xmm8
    movdqu [rsp+128], xmm7
    movdqu [rsp+144], xmm6
    movdqu [rsp+160], xmm5
    movdqu [rsp+176], xmm4
    movdqu [rsp+192], xmm3
    movdqu [rsp+208], xmm2
    movdqu [rsp+224], xmm1
    movdqu [rsp+240], xmm0
%endmacro

%macro restore_context 0
    movdqu xmm15, [rsp]
    movdqu xmm14, [rsp+16]
    movdqu xmm13, [rsp+32]
    movdqu xmm12, [rsp+48]
    movdqu xmm11, [rsp+64]
    movdqu xmm10, [rsp+80]
    movdqu xmm9, [rsp+96]
    movdqu xmm8, [rsp+112]
    movdqu xmm7, [rsp+128]
    movdqu xmm6, [rsp+144]
    movdqu xmm5, [rsp+160]
    movdqu xmm4, [rsp+176]
    movdqu xmm3, [rsp+192]
    movdqu xmm2, [rsp+208]
    movdqu xmm1, [rsp+224]
    movdqu xmm0, [rsp+240]
    add rsp, 256
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp
%endmacro

%macro restore_context_light 0
    add rsp, 256
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp
%endmacro

%endif