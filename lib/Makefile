all: libhelpers.so

clean:
	rm -f helpers.o
	rm -f libhelpers.so

helpers.o: helpers.c
	gcc -o $@ -c -Wall -fpic -I. $<

libhelpers.so: helpers.o
	gcc -o $@ -shared $<
