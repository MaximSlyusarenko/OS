all: simplesh

clean:
	rm -f simplesh.o
	rm -f simplesh
	$(MAKE) -C ../lib clean

simplesh: simplesh.o ../lib/libhelpers.so ../lib/libbufio.so
	gcc -o $@ -L../lib simplesh.o ../lib/libhelpers.so -lhelpers ../lib/libbufio.so -lbufio

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

../lib/libbufio.so:	
	$(MAKE) -C $(dir $@) $(notdir $@)

simplesh.o: simplesh.c
	gcc -std=c99 -o $@ -c -Wall -I../lib $<
