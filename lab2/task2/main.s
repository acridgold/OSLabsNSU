.section .data
message:
    .ascii "Hello world\n"

.section .text
.globl _start
_start:
    # write(1, message, 12)
    movq $1, %rax             # sys_write
    movq $1, %rdi             # stdout
    lea message, %rsi         # адрес строки
    movq $12, %rdx            # длина строки
    syscall

    # exit(0)
    movq $60, %rax            # sys_exit
    xorq %rdi, %rdi           # код возврата 0
    syscall