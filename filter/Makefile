all: filter

clean:
	rm -f filter.o
	rm -f filter
	rm -f err.txt
	$(MAKE) -C ../lib clean

filter: filter.o ../lib/libhelpers.so
	gcc -o $@ filter.o -Wl,-rpath=../lib -L../lib -lhelpers

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

filter.o: filter.c
	gcc -std=c99 -o $@ -c -Wall -I../lib $<
