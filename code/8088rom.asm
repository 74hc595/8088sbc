; vim:noet:sw=8:ts=8:sts=8:ai:syn=nasm
        cpu 8086

ROMSIZE equ 32768
ROMCS   equ 0F000h
INIT_SS equ 07000h
INIT_SP equ 0FFF0h

        org 10000h-ROMSIZE

%include "macros.inc"
%include "api.inc"
%include "uart.inc"
%include "display.inc"
%include "banksel.inc"
%include "gpios.inc"
%include "output.inc"

SKIP_SELF_TEST equ 0

; ------------------------------------------------------------------------------
start:
        ; initialize stack pointer
        mov     ax, INIT_SS     ; initialize stack pointer
        mov     bx, INIT_SP
        mov     ss, ax
        mov     sp, bx
        cld
%if SKIP_SELF_TEST
        jmp     init
%endif

; ------------------------------------------------------------------------------
; Self-test.
; First, test the lower 512K of RAM. (0000:0000-7000:FFFF)
; Then, copy this routine to lower RAM, bank out the ROM and bank in upper RAM,
; and test the upper 512K of RAM.
; Don't use the stack until lower RAM has been verified.
lower_ram_test:
        clr     si
        mov     ds, si
        jmp     test_segment
; Test a 512K chunk of memory at DS:0000.
ram_test_512k:
        banksel URAM    ; (not executed when testing lower RAM)
test_segment:
        clr     si
        clr     di
        mov     ax, ds
        mov     es, ax
        out1_tgl
; write test pattern 1
        mov     ax, 55AAh
        mov     cx, 32768
        rep stosw
; verify test pattern 1
        mov     cx, 32768
.1:     lodsw
        cmp     ax, 55AAh
        jne     .fail
        loop    .1
; write test pattern 2
        mov     ax, 0AA55h
        mov     cx, 32768
        rep stosw
; verify test pattern 2
        mov     cx, 32768
.2:     lodsw
        cmp     ax, 0AA55h
        jne     .fail
        loop    .2
; advance to next page
        mov     ax, ds
        add     ax, 1000h
        jz      ram_test_end   ; upper ram test ends when 0xF000 wraps to 0x0000 
        jo      upper_ram_test ; lower ram test ends once 0x8000 is reached
        mov     ds, ax
        jmp     test_segment
.fail:
; flash leds forever
        out01_tgl
        clr     cx
        loop    $
        jmp     .fail
ram_test_end:
; test passed, return to ROM
        banksel UROM
        retf
ram_test_512k_end:

upper_ram_test:
; turn on orange LED
        out0_set
; We will be banking out the ROM, so relocate the test function to lower RAM.
        clr     di                      ; destination is 0000:0000
        mov     es, di
        mov     ax, cs                  ; source segment is ROM (0xF000)
        mov     ds, ax
        mov     si, ram_test_512k       ; source address
        mov     cx, ram_test_512k_end-ram_test_512k
; copy test code to lower RAM
        rep movsb
; begin memory test at 8000:0000
        mov     ax, 8000h
        mov     ds, ax
        clr     si
; verified that lower RAM works, so we can use the stack to call
        call    0000:0000
; if we get here, the test passed.
        jmp     init
; ------------------------------------------------------------------------------
crash:
        cli
; flash leds forever
.1:     out01_tgl
        mov     cx, 4000h
        loop    $               ; delay loop
        jmp     .1
; ------------------------------------------------------------------------------
div0:
        push    si
        mov     si, str_div0
        jmp     dump_regs
singlestep:
        push    si
        mov     si, str_singlestep
        jmp     dump_regs
nmi:
        push    si
        mov     si, str_nmi
        jmp     dump_regs
breakpt:
        push    si
        mov     si, str_breakpt
        jmp     dump_regs
overflow:
        push    si
        mov     si, str_overflow
        jmp     dump_regs
int05h:
        push    si
        mov     si, 5
        jmp     dump_regs
; ------------------------------------------------------------------------------
; Expected to be called via an interrupt vector.
; SI - pointer to interrupt name/number
; [SP] - previous SI value
; [SP+2] - caller's IP
; [SP+4] - caller's CS
; [SP+6] - flags
dump_regs:
        push    sp              ; SP value to be printed
        push    bp
        mov     bp, sp
        push    word [bp+2]     ; previous SP
        push    ss
        push    dx
        push    word [bp+10]    ; flags (lower byte)
        push    di
        push    es
        push    cx
        push    word [bp+10]    ; flags (upper byte)
        push    word [bp+4]     ; previous SI
        push    ds
        push    bx
        push    si              ; interrupt number
        push    word [bp+6]     ; IP of caller
        push    word [bp+8]     ; CS of caller
        push    ax
        mov     si, str_regdump
        PRINTF
        mov     sp, bp
        pop     bp
        add     sp, 2
        pop     si
        iret
; ------------------------------------------------------------------------------
initial_ivt:
; Processor reserved vectors
        dw      div0, ROMCS             ; INT 00h: divide error
        dw      singlestep, ROMCS       ; INT 01h: single step
        dw      nmi, ROMCS              ; INT 02h: NMI
        dw      breakpt, ROMCS          ; INT 03h: breakpoint
        dw      overflow, ROMCS         ; INT 04h: overflow
        dw      int05h, ROMCS           ; INT 05h: reserved (bounds check on 80186)
; API vectors
        dw      disp_outch_int, ROMCS   ; INT 06h: character output
        dw      puts_int, ROMCS         ; INT 07h: string output
        dw      printf_int, ROMCS       ; INT 08h: formatted string output
initial_ivt_end:
; ------------------------------------------------------------------------------
init:
; Initialize the serial port at 115200 baud.
        call    uart_init_115200
; Initialize the display.
        call    disp_init

; Load the interrupt vector table.
; Default character output device is the serial port.
        mov     si, initial_ivt ; source: initial vector table in ROM
        mov     ax, cs
        mov     ds, ax
        clr     di              ; destination: 0000:0000
        mov     es, di
        mov     cx, (initial_ivt_end-initial_ivt)/2
        rep movsw
        sti                     ; interrupts enabled! we're live!
; print welcome message
        mov     si, str_welcome
        PUTS
; light both LEDs at half brightness
.1:     out01_tgl
        jmp     .1

str_welcome:
        db      'Hello 8088!', 0Ah, 0
str_div0:
        db      ' DIV ERR', 0
str_singlestep:
        db      ' S. STEP', 0
str_nmi:
        db      '     NMI', 0
str_breakpt:
        db      ' BREAKPT', 0
str_overflow:
        db      'OVERFLOW', 0
str_int05h:
        db      ' INT 05H', 0
str_regdump:
        db      FF
        db      'AX=', FMT_H16, ' CS=', FMT_H16, ' IP=', FMT_H16, ' ', FMT_SCS, 0Ah,
        db      'BX=', FMT_H16, ' DS=', FMT_H16, ' SI=', FMT_H16, ' ', FMT_FLH, 0Ah,
        db      'CX=', FMT_H16, ' ES=', FMT_H16, ' DI=', FMT_H16, ' ', FMT_FLL, 0Ah
        db      'DX=', FMT_H16, ' SS=', FMT_H16, ' SP=', FMT_H16, ' BP=', FMT_H16, 0Ah, 0

str_foo:
        db      FF, FMT_H8, 0Ah, 0

; ------------------------------------------------------------------------------
; pad out the ROM with FFh
times ROMSIZE-16-($-$$) db 0FFh
; reset vector
        jmp     ROMCS:start
; pad out the rest of the ROM with NOPs
times ROMSIZE-($-$$) db 0x90
