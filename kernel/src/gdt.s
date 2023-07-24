extern gdtr
global gdt_load

gdt_load:
    lgdt [gdtr]
    push 0x08
    mov rax, .reload_segments
    push rax
    retfq
.reload_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
