; Routines for a matrix of PDSP-1881/HDSP-2xxx intelligent LED displays.
; (up to 128 characters)

; Bits 0-2: character index in display
; Bits 3-6: display module index
DISP_IOBASE     equ     0100h
DISP_NCHARS     equ     128
DISP_NCOLS      equ     32
DISP_NROWS      equ     DISP_NCHARS/DISP_NCOLS
DISP_CPM        equ     8                       ; characters per module
DISP_NMODULES   equ     DISP_NCHARS/DISP_CPM
DISP_FLASH      equ     DISP_IOBASE|0000h       ; Flash RAM
DISP_UDC_ADDR   equ     DISP_IOBASE|0800h       ; UDC Address Register
DISP_UDC_RAM    equ     DISP_IOBASE|0A00h       ; UDC RAM
DISP_CONTROL    equ     DISP_IOBASE|0C00h       ; Control Register
DISP_CHARS      equ     DISP_IOBASE|0E00h       ; Character RAM

; The "cursor position" is stored in the UART scratchpad register.
DISP_CURSORPOS  equ     UART_SPR

; Set the control register of all display modules to AL.
; Clobbers: CX, DX
disp_set_controlreg:
        mov     dx, DISP_CONTROL
        mov     cx, DISP_NMODULES
.1      out     dx, al
        add     dx, DISP_CPM
        loop    .1
        ret

; Initialize and clear the display to 53% brightness.
disp_init:
        mov     al, 0FFh
        out     DISP_CURSORPOS, al
        mov     al, 8Ah
        jmp     disp_set_controlreg

; Vectored UART character handler.
disp_outch_int:
        push    ax
        push    dx
        mov     ah, al                  ; save character to be written
        in      al, DISP_CURSORPOS      ; get cursor pos
        mov     dl, al                  ; form port address in DX
        mov     dh, DISP_CHARS>>8
        mov     al, ah                  ; get character to be written
        cmp     al, 0Ah                 ; newline character?
        je      .nl
        cmp     al, 0Ch                 ; form feed (clear screen)?
        je      .ff
        inc     dl                      ; advance cursor position (low byte only)
        and     dl, DISP_NCHARS-1       ; wrap around
        out     dx, al                  ; write character
.stpos: mov     al, dl                  ; save new cursor pos
        out     DISP_CURSORPOS, al
.done:  pop     dx
        pop     ax
        iret
; newline
.nl:    mov     al, dl                  ; already at the last column?
        and     al, DISP_NCOLS-1
        cmp     al, DISP_NCOLS-1
        je      .done                   ; if so, don't do anything
        or      dl, DISP_NCOLS-1        ; move to end of line
        jmp     .stpos
; formfeed (clear screen)
.ff:    push    cx
        mov     dx, DISP_CONTROL        ; read value from control register
        in      al, dx
        or      al, 80h                 ; set 'reset display' bit
        call    disp_set_controlreg     ; clear all displays
        pop     cx
        mov     dl, 0FFh                ; reset cursor pos
        jmp     .stpos





