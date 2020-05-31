; INT 06h: print character in AL
%macro PUTC 0
        int     06h
%endmacro

; INT 07h: print string in CS:SI
%macro PUTS 0
        int     07h
%endmacro

; INT 08h: print formatted string in CS:SI, arguments on stack (caller cleanup)
%macro PRINTF 0
        int     08h
%endmacro
