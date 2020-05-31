# 8088 Single Board Computer

A minimal single-board computer based on the Intel 8088 processor.
That's the same processor used in the original IBM PC, but this is just a
simple project and doesn't strive for PC/DOS compatibility.

![](https://pbs.twimg.com/media/EYRXZCHUYAA31Lq?format=jpg&name=large)

## Specs
- 8088 or compatible CPU in minimal mode (5 MHz with [8088](https://www.jameco.com/z/8088-Major-Brands-IC-8088-8-Bit-HMOS-MPU_52142.html), 8 MHz with 8088-2)
    - Clock oscillator at 3x frequency of CPU clock frequency. For 8 MHz operation, uses a [24 MHz half-can crystal oscillator](https://www.mouser.com/ProductDetail/ECS/ECS-2200B-240?qs=GxOUx7aO6nwQDraMtk%2FyHw%3D%3D)
- 1 MB static RAM (2x [AS6C4008-55PCN](https://www.mouser.com/ProductDetail/Alliance-Memory/AS6C4008-55PCN?qs=%2Fha2pyFaduhSIt5XfNvHc%2FWglHK486gNG4HA%2F19hDPofxZgoFVNuaw%3D%3D))
- 32 KB ROM (1x [AT28C256-15](https://www.jameco.com/z/28C256-15-Major-Brands-IC-28C256-16-EEPROM-256K-Bit-CMOS-Parallel_74843.html))
- FTDI-compatible 5V serial port provided by a [16C550](https://www.jameco.com/shop/ProductDisplay?catalogId=10001&langId=-1&storeId=10001&productId=288809) UART (up to 115200 baud)
- 32 column by 4 row LED display (sixteen PDSP-1881 intelligent displays,
  display board schematic available [here](https://github.com/74hc595/kicad-intelligent-displays))
- Uses five common 74HC chips for glue logic (74HC4017, 74HC132, 74HC573,
  74HC75, 74HC139). Does not require any of the Intel 8200-series support chips.
- 5V power via USB-A connector

## Memory map

```
00000h-7FFFFh - RAM
80000h-FFFFFh - bankable (RAM, ROM, expansion 1, or expansion 2)
```

## I/O map

Only bits 7 and 8 are used for decoding.

```
000h-07Fh - I/O bank 0
    060h-067h - 16550 UART
080h-0FFh - I/O bank 1
100h-17Fh - I/O bank 2
    100h-107h - 128-character display (16x PDSP-1881 or similar)
180h-1FFh - I/O bank 3
```

More info on display decoding:

```
100h-17Fh - Flash RAM
900h-97Fh - UDC Address Register
B00h-B7Fh - UDC RAM
D00h-D7Fh - Control Word Register
F00h-F7Fh - Character RAM
```

## Bank switching and input/output

The modem control lines of the 16550 UART are used as general purpose input and
output lines.

```
~OUT1 - bank select bit 0
~OUT2 - bank select bit 1
~DTR  - general purpose output 0 (LED2)
~RTS  - general purpose output 1 (LED3)
~RI   - general purpose input 0
~DCD  - general purpose input 1
~DSR  - general purpose input 2
~CTS  - general purpose input 3
```

Bank select bits 0 and 1 control which device is mapped into the upper half of
the address space:

```
00 - expansion 1
01 - expansion 2
10 - RAM
11 - ROM (default at power-up)
```

These are controlled by bits 2 and 3 of the 16550's Modem Control Register.
(`64h`)

## Interrupts

The four general-purpose inputs can be set up as inputs by configuring the
16550's Interrupt Enable Register (`61h`). However, this won't be very useful
because the 8088's `~INTA` line is not connected to anything, so when an
interrupt is received, there is no device to place the vector number on the data
bus.

Pressing the `NMI` button will generate a non-maskable interrupt (`INT 04h`).
The button is debounced in hardware.

## Software

Not much here, just enough to test memory and dump the register state. Compiles
with NASM (version 2.14.02 used). Running `make` in the `code/` directory
generates as 32KB binary file. Running `make flash` can be used to burn the
ROM image to an AT28C256 EEPROM chip using a [MiniPro TL866II+](https://www.amazon.com/tl866ii-plus/s?k=tl866ii+plus)
with the [minipro](https://gitlab.com/DavidGriffith/minipro/) utility.

(The TL866II+ is around $60 USD and widely available, if you don't have one,
you should get one!)

1. Power LED will light up.
2. `OUT1` LED will blink four times. (Testing 8 segments of lower RAM)
3. `OUT0` will light.
4. `OUT1` LED will blink four times. (Testing 8 segments of upper RAM)
5. `Hello 8088!` will be printed to the serial port. (115200 8N1)
6. `OUT0` and `OUT1` LEDs will toggle with 50% duty cycle. (verify with scope)
7. Pressing the `NMI` button will print the register state:

```
AX=F001 CS=F000 IP=82E0      NMI
BX=FFF0 DS=F000 SI=82F5     --I-
CX=0000 ES=0000 DI=0024 --------
DX=F202 SS=7000 SP=FFE6 BP=FFFF
```

### API

Interrupt vector `06h` points to the character output routine. If using the
PDSP-1881 LED display, this can be changed to `disp_outch_int` to write to the
display instead of the serial port.

`INT 07h` prints the null-terminated string in `CS:SI`.

`INT 08h` is a rudimentary formatted-output routine. See `output.inc`.

## Future work

Probably none. This was just a quick project and I probably won't work on it
long-term because... honestly, I'm just not a fan of 8086 assembly language and
its weird 16-bit segmented model. Mainly, I had these chips lying around and
wanted to do something with them.

## About

Open hardware, released under the terms of the 3-clause BSD license.

Copyright 2020 Matt Sarnoff.

http://twitter.com/txsector

http://msarnoff.org
