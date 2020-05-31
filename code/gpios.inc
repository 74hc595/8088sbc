; Control of onboard general purpose input and output pins.
; These are driven by the 16550 UART.
; All clobber AL.
;
; OUT0: ~DTR (orange LED)
; OUT1: ~RTS (blue LED)
; IN0:  ~RI
; IN1:  ~DCD
; IN2:  ~DSR
; IN3:  ~CTS

%macro out_set 1
        in      al, UART_MCR
        or      al, %1
        out     UART_MCR, al
%endmacro

%macro out_clr 1
        in      al, UART_MCR
        and     al, ~%1
        out     UART_MCR, al
%endmacro

%macro out_tgl 1
        in      al, UART_MCR
        xor     al, %1
        out     UART_MCR, al
%endmacro

%macro out0_set 0
  out_set 01b
%endmacro

%macro out0_clr 0
  out_clr 01b
%endmacro

%macro out0_tgl 0
  out_tgl 01b
%endmacro

%macro out1_set 0
  out_set 10b
%endmacro

%macro out1_clr 0
  out_clr 10b
%endmacro

%macro out1_tgl 0
  out_tgl 10b
%endmacro

%macro out01_set 0
  out_set 11b
%endmacro

%macro out01_clr 0
  out_clr 10b
%endmacro

%macro out01_tgl 0
  out_tgl 11b
%endmacro

