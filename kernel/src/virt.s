global set_cr3
global get_cr3
global invlpg

set_cr3:
    mov cr3, rdi
    ret

get_cr3:
    mov rax, cr3
    ret

invlpg:
    invlpg [rdi]
    ret
