PROGRAM = 8088rom

BIN = $(PROGRAM).bin
LST = $(PROGRAM).lst

all: $(BIN)

$(BIN): $(PROGRAM).asm *.inc
	nasm -o $(BIN) $(PROGRAM).asm

clean:
	rm -f $(BIN) $(LST)

flash: $(BIN)
	minipro -p AT28C256 -w $(BIN)
