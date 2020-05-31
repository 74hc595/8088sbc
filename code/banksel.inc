; Upper memory bank selection.
; Controls which bank of memory is present in the upper 512KB of address space.
;
; Bank selection is controlled by the ~OUT1 and ~OUT2 lines of the 16550 UART,
; bits 2 and 3 of the Modem Control Register.
;
; Bank 0: Upper ROM
; Bank 1: Upper RAM
; Bank 2: Expansion 2
; Bank 3: Expansion 1

UROM    equ 0
URAM    equ 1
EXP2    equ 2
EXP1    equ 3

; Clobbers: AL
%macro banksel 1
        in      al, UART_MCR
%if %1 == 0
        and     al, 11110011b
%elif %1 == 1
        and     al, 11110011b
        or      al, 00000100b
%elif %1 == 2
        and     al, 11110011b
        or      al, 00000100b
%elif %1 == 3
        or      al, 00001100b
%else
%error "Invalid bank number"
%endif
        out     UART_MCR, al
%endmacro