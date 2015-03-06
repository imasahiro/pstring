
all: pstring.c pstring.h
	$(CC) -O3 -march=native pstring.c -o pstring

clean:
	-rm pstring
