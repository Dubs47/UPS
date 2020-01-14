IDIR=src
CC=gcc
BIN=server
CFLAGS=-I$(IDIR)

ODIR=src
LDIR=src

LIBS=-lm -pthread

_DEPS = structs.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = game.o game_logic.o player.o connection.o main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	mv -f $(ODIR)/*.o *~ core $(INCDIR)/*~
