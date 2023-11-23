CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: encode decode

encode: common.c encode.c
	$(CC) $(CFLAGS) -o $@ $^

decode: common.c decode.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f encode decode *.o
