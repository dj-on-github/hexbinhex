CC = gcc
CFLAGS = -I/usr/local/include -m64 -g -Wall
LDFLAGS = -L/usr/local/lib 
LDLIBS = -lm

all: hex2bin bin2hex bin201 bin2nistoddball nistoddball2bin 012bin bin2dec

nistoddball2bin: nistoddball2bin.c
	$(CC) $(CFLAGS) $(LDFLAGS) nistoddball2bin.c  -o nistoddball2bin $(LDLIBS)

bin2nistoddball: bin2nistoddball.c
	$(CC) $(CFLAGS) $(LDFLAGS) bin2nistoddball.c  -o bin2nistoddball $(LDLIBS)

hex2bin: hex2bin.c
	$(CC) $(CFLAGS) $(LDFLAGS) hex2bin.c  -o hex2bin $(LDLIBS)

bin2hex: bin2hex.c
	$(CC) $(CFLAGS) $(LDFLAGS) bin2hex.c  -o bin2hex $(LDLIBS)

bin201: bin201.c
	$(CC) $(CFLAGS) $(LDFLAGS) bin201.c  -o bin201 $(LDLIBS)

012bin: 012bin.c
	$(CC) $(CFLAGS) $(LDFLAGS) 012bin.c  -o 012bin $(LDLIBS)

bin2dec: bin2dec.c
	$(CC) $(CFLAGS) $(LDFLAGS) bin2dec.c  -o bin2dec $(LDLIBS)

install: bin2hex bin201 hex2bin bin2nistoddball nistoddball2bin
	cp bin2hex /usr/local/bin
	cp bin201  /usr/local/bin
	cp 012bin  /usr/local/bin
	cp hex2bin /usr/local/bin
	cp bin2nistoddball /usr/local/bin
	cp nistoddball2bin /usr/local/bin
	cp bin2dec /usr/local/bin
clean:
	rm bin2hex
	rm bin201
	rm 012bin
	rm hex2bin
	rm bin2nistoddball
	rm nistoddball2bin
	rm bin2dec



