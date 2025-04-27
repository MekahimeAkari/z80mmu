.POSIX:
.SUFFIXES:
.PHONY: clean distclean

SRC = main.c
OBJ = $(SRC:.c=.o)
EXE = z80sim
ASM = test.asm
BIN = test.bin

include config.mk

all: $(EXE) $(BIN)

$(BIN): $(ASM)
	z80asm $(ASM) -o $(BIN)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(EXE)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

distclean: clean
	rm -f config.mk compile_commands.json
clean:
	rm -f $(EXE) $(OBJ) $(BIN)
