# --------------------------------------------------------------
# Makefile – Othello for Picocomputer‑6502 (CC65)
# --------------------------------------------------------------

CC      = cl65
CFLAGS  = -t picocomputer -Os -Cl
LDFLAGS = -t picocomputer
SRC     = src/main.c src/gfx.c src/input.c src/othello.c
OBJ     = $(SRC:.c=.o)

TARGET  = othello.bin

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) --binary

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
