all: filter

clean:
	rm -f filter
	rm -f err.txt
	$(MAKE) -C ../lib clean

filter: ../lib/libhelpers.so
	gcc -o $@ filter.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
