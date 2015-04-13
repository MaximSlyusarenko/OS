all: buffilter

clean:
	rm -f buffilter.o
	rm -f buffilter
	rm -f err.txt
	rm -f buffilter2.c
	$(MAKE) -C ../lib clean

buffilter: buffilter.o ../lib/libbufio.so ../lib/libhelpers.so
	gcc -o $@ -L../lib buffilter.o ../lib/libbufio.so ../lib/libhelpers.so -lbufio -lhelpers

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

buffilter.o: buffilter.c
	gcc -o $@ -c -Wall -I../lib $<

