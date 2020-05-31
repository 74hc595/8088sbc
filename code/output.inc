; Text output routines.

; ASCII control characters
NL      equ     0Ah     ; newline
FF      equ     0Ch     ; form feed

; Print a null-terminated string from ROM.
; Arguments: pointer to string in CS:SI
puts_int:
        push    ax
        push    ds
        mov     ax, cs
        mov     ds, ax
.1:     lodsb           ; get character
        or      al, al  ; end if 0
        jz      .done
        PUTC
        jmp     .1
.done:  pop     ds
        pop     ax
        iret


; Print AL as two hexadecimal digits. (0-9, A-F)
; Clobbers: FLAGS
print_hexbyte:
        push    ax
        mov     ah, al  ; save a copy for lower nibble
times 4 shr     al, 1   ; shift down upper nibble
        add     al, '0' ; convert to ASCII
        cmp     al, '9' ; >= 10?
        jna     .1
	add     al, 'A'-'9'-1
.1:     PUTC
        mov     al, ah 
        and     al, 0Fh ; isolate lower nibble
        add     al, '0'
        cmp     al, '9'
        jna     .2
	add     al, 'A'-'9'-1
.2:     PUTC
        pop     ax
        ret

; Print AX as two hexadecimal digits.
; Clobbers: FLAGS
print_hexword:
        xchg    al, ah
        call    print_hexbyte
        xchg    al, ah
        jmp     print_hexbyte

; Print DX:AX as two hexadecimal digits.
; Clobbers: FLAGS
print_hexdword:
        xchg    ax, dx
        call    print_hexword
        xchg    ax, dx
        jmp     print_hexword

; Print a formatted string from ROM.
; Unlike C printf, format specifiers are bytes in the range F8h-FFh.
; Arguments: pointer to format string in CS:SI
;            arguments to be formatted on stack
;            (caller is responsible for cleaning up the stack)
FMT_C8  equ     0F8h    ; low byte as ASCII character
FMT_H8  equ     0F9h    ; low byte of word as hexadecimal (2 digits)
FMT_H16 equ     0FAh    ; word as hexadecimal (4 digits)
FMT_H32 equ     0FBh    ; doubleword as hexadecimal (8 digits)
FMT_SCS equ     0FCh    ; pointer to null-terminated CS-relative string
FMT_SDS equ     0FDh    ; pointer to null-terminated DS-relative string
FMT_FLL equ     0FEh    ; low byte as flag image (ZF, AF, PF, CF)
FMT_FLH equ     0FFh    ; hight byte as flag image (OF, DF, IF, TF)
printf_int:
        push    bp
        mov     bp, sp
        add     bp, 8   ; bp points to first vararg
        push    ax
        push    ds
        mov     ax, cs
        mov     ds, ax

.1:     lodsb           ; get character
        or      al, al  ; end if 0
        jz      .done
        cmp     al, FMT_C8
        jb      .nofmt
        je      .fmt_c8
        cmp     al, FMT_H8
        je      .fmt_h8
        cmp     al, FMT_H16
        je      .fmt_h16
        cmp     al, FMT_H32
        je      .fmt_h32
        cmp     al, FMT_SCS
        je      .fmt_scs
        cmp     al, FMT_FLL
        je      .fmt_fll
        cmp     al, FMT_FLH
        je      .fmt_flh
.nofmt: PUTC
        jmp     .1
.done:  pop     ds
        pop     ax
        pop     bp
        iret

.fmt_c8:
        mov     al, [bp]
        add     bp, 2
        jmp     .nofmt
.fmt_h8:
        mov     al, [bp]
        add     bp, 2
        call    print_hexbyte
        jmp     .1
.fmt_h16:
        mov     ax, [bp]
        add     bp, 2
        call    print_hexword
        jmp     .1
; push high word first
.fmt_h32:
        mov     ax, [bp]
        mov     dx, [bp+2]
        add     bp, 4
        call    print_hexdword
        jmp     .1
.fmt_scs:
        push    si
        mov     si, [bp]
        add     bp, 2
        PUTS
        pop     si
        jmp     .1
.fmt_fll:
        mov     dx, [bp]
        add     bp, 2
; sign flag
        mov     al, '-'
        test    dl, 80h
        jz      .fl1
        mov     al, 'S'
.fl1:   PUTC
; zero flag
        mov     al, '-'
        test    dl, 40h
        jz      .fl2
        mov     al, 'Z'
.fl2:   PUTC
; bit 5 reserved
        mov     al, '-'
        PUTC
; auxiliary carry flag
        mov     al, '-'
        test    dl, 10h
        jz      .fl3
        mov     al, 'A'
.fl3:   PUTC
; bit 3 reserved
        mov     al, '-'
        PUTC
; parity flag
        mov     al, '-'
        test    dl, 04h
        jz      .fl4
        mov     al, 'P'
.fl4:   PUTC
; bit 1 reserved (always 1)
        mov     al, '-'
        PUTC
; carry flag
        mov     al, '-'
        test    dl, 01h
        jz      .nofmt
        mov     al, 'C'
        jmp     .nofmt

.fmt_flh:
        mov     dx, [bp]
        add     bp, 2
; pad upper 4 bits with spaces since they're not used
        mov     al, ' '
        PUTC
        PUTC
        PUTC
        PUTC
; overflow flag
        mov     al, '-'
        test    dh, 08h
        jz      .fh1
        mov     al, 'O'
.fh1:   PUTC
; direction flag
        mov     al, '-'
        test    dh, 04h
        jz      .fh2
        mov     al, 'D'
.fh2:   PUTC
; interrupt flag
        mov     al, '-'
        test    dh, 02h
        jz      .fh3
        mov     al, 'I'
.fh3:   PUTC
; trap flag
        mov     al, '-'
        test    dh, 01h
        jz      .nofmt
        mov     al, 'T'
        jmp     .nofmt
