%include "src/arch/x86_64/common.asm"

[extern isr_handler]

%macro create_isr 1
[global isr%1]
isr%1:
  push %1
  push rbp

  jmp isr_common_handler
%endmacro

%macro create_isr_dummy 1
[global isr%1]
isr%1:
  cli

  push 0
  push %1
  push rbp

  jmp isr_common_handler
%endmacro

create_isr_dummy 0
create_isr_dummy 1
create_isr_dummy 2
create_isr_dummy 3
create_isr_dummy 4
create_isr_dummy 5
create_isr_dummy 6
create_isr_dummy 7
create_isr 8
create_isr_dummy 9
create_isr 10
create_isr 11
create_isr 12
create_isr 13
create_isr 14
create_isr_dummy 15
create_isr_dummy 16
create_isr_dummy 17
create_isr_dummy 18
create_isr_dummy 19
create_isr_dummy 20
create_isr_dummy 21
create_isr_dummy 22
create_isr_dummy 23
create_isr_dummy 24
create_isr_dummy 25
create_isr_dummy 26
create_isr_dummy 27
create_isr_dummy 28
create_isr_dummy 29
create_isr_dummy 30
create_isr_dummy 31

isr_common_handler:

  call isr_handler

  add rsp, 16

  iretq
