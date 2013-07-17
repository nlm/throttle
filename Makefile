CC=gcc
CFLAGS=-Wall -ansi -pedantic
LDFLAGS=
THROTTLE_BIN=throttle
THROTTLE_OBJ=throttle.o
THROTTLE_SRC=throttle.c
TDUMP_BIN=tdump
TDUMP_OBJ=tdump.o
TDUMP_SRC=tdump.c


all: $(THROTTLE_BIN) $(TDUMP_BIN)

$(THROTTLE_BIN): $(THROTTLE_OBJ)
	$(CC) $(LDFLAGS) $(THROTTLE_OBJ) -o $(THROTTLE_BIN)
	strip -s $(THROTTLE_BIN)

$(TDUMP_BIN): $(TDUMP_OBJ)
	$(CC) $(LDFLAGS) $(TDUMP_OBJ) -o $(TDUMP_BIN)
	strip -s $(TDUMP_BIN)

clean:
	rm -f $(THROTTLE_OBJ) $(TDUMP_OBJ)

distclean: clean
	rm -f $(THROTTLE_BIN) $(TDUMP_BIN)
