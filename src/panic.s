global get_return_address
global get_previous_rbp

; uint64_t get_return_address(uint64_t rbp)
get_return_address:
    mov rax, [rdi + 8]
    ret

; uint64_t get_previous_rbp(uint64_t rbp)
; if rbp == 0, current rbp is used
get_previous_rbp:
    cmp rdi, 0
    je .current_rbp
    mov rax, [rdi]
    ret
.current_rbp:
    mov rax, rbp
    ret
