
all: pstring.c pstring.h
	$(CC) -O3 -march=native pstring.c -o pstring

test:
	./pstring

clean:
	-rm pstring
