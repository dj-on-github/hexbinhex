CC = gcc
CFLAGS = -I/usr/local/include -m64 -g -Wall
LDFLAGS = -L/usr/local/lib 
LDLIBS = -lm

all: hex2bin bin2hex bin2nistoddball

bin2nistoddball: bin2nistoddball.c
	$(CC) $(CFLAGS) $(LDFLAGS) bin2nistoddball.c  -o bin2nistoddball $(LDLIBS)

hex2bin: hex2bin.c
	$(CC) $(CFLAGS) $(LDFLAGS) hex2bin.c  -o hex2bin $(LDLIBS)

bin2hex: bin2hex.c
	$(CC) $(CFLAGS) $(LDFLAGS) bin2hex.c  -o bin2hex $(LDLIBS)

install:
	cp bin2hex /usr/local/bin
	cp hex2bin /usr/local/bin
	cp bin2nistoddball /usr/local/bin

clean:
	rm bin2hex
	rm hex2bin
	rm bin2nistoddball


