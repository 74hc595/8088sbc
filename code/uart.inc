; 16550 UART
UART_IOBASE     equ 060h
UART_RHR        equ UART_IOBASE+0       ; receive holding register (read)
UART_THR        equ UART_IOBASE+0       ; transmit holding register (write)
UART_IER        equ UART_IOBASE+1       ; interrupt enable register
UART_ISR        equ UART_IOBASE+2       ; interrupt status register (read)
UART_FCR        equ UART_IOBASE+2       ; FIFO control register (write)
UART_LCR        equ UART_IOBASE+3       ; line control register
UART_MCR        equ UART_IOBASE+4       ; modem control register
UART_LSR        equ UART_IOBASE+5       ; line status register
UART_MSR        equ UART_IOBASE+6       ; modem status register
UART_SPR        equ UART_IOBASE+7       ; scratchpad register (reserved for system use)
UART_DLL        equ UART_IOBASE+0       ; divisor latch LSB
UART_DLM        equ UART_IOBASE+1       ; divisor latch MSB
; aliases for register names (used by different manufacturers)
UART_RBR        equ UART_RHR            ; receive buffer register
UART_IIR        equ UART_ISR            ; interrupt identification register
UART_SCR        equ UART_SPR            ; scratch register

; Frequency of the UART's input clock.
UART_CLK        equ 1843200

; If 1, ~TXRDY is connected to ~TEST, and the WAIT instruction can be used to
; check if the UART's transmit FIFO has space available.
nTXRDY_IS_nTEST equ 1

; Initialize the UART.
; 115200 baud, 8 data bits, no parity, 1 stop bit.
uart_init_115200:
        mov     ax, 0900h
        out     UART_IER, ax    ; disable interrupts and configure FIFO
        mov     al, 10000011b   ; 8 data bits, no parity, 1 stop bit, DLAB=1
        out     UART_LCR, al
        mov     ax, UART_CLK/(115200*16)
        out     UART_DLL, ax
        mov     al, 00000011b   ; disable divisor latch
        out     UART_LCR, al
        ret

; Write the character in AL to the serial port.
; Clobbers: AH
uart_outch:
; wait until the transmit holding register is empty
%if nTXRDY_IS_nTEST
        wait
%else
.wait:  mov     ah, al
        in      al, UART_LSR
        test    al, 00100000b
        jz      .wait
        mov     al, ah
%endif
        out     UART_THR, al
        ret

; Vectored UART character handler.
; Clobbers: none
uart_outch_int:
%if nTXRDY_IS_nTEST
        wait
%else
        push    ax
.wait:  in      al, UART_LSR
        test    al, 00100000b
        jz      .wait
        pop     ax
%endif
        out     UART_THR, al
        iret
