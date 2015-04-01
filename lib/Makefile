all: libhelpers.so libbufio.so

clean:
	rm -f helpers.o
	rm -f libhelpers.so
	rm -f libbufio.so
	rm -f bufio.o

helpers.o: helpers.c
	gcc -o $@ -c -Wall -fpic -I. $<

libhelpers.so: helpers.o
	gcc -o $@ -shared $<

bufio.o: bufio.c
	gcc -o $@ -c -Wall -fpic -I. $<
	
libbufio.so: bufio.o
	gcc -o $@ -shared $<		
