%include "src/arch/x86_64/common.asm"

[extern irq_handler]

%macro create_irq 1
[global irq%1]
irq%1:
  push rax
  push %1

jmp irq_common_handler
%endmacro

create_irq 0
create_irq 1
create_irq 2
create_irq 3
create_irq 4
create_irq 5
create_irq 6
create_irq 7
create_irq 8
create_irq 9
create_irq 10
create_irq 11
create_irq 12
create_irq 13
create_irq 14
create_irq 15

irq_common_handler:
  save_context

  ;restore_kernel_segments

  mov rdi, rsp
  call irq_handler

  restore_context

  add rsp, 16

  iretq
