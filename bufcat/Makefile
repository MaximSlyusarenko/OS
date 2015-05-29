all: bufcat

clean:
	rm -f bufcat.o
	rm -f bufcat
	rm -f err.txt
	rm -f bufcat2.c
	$(MAKE) -C ../lib clean

bufcat: bufcat.o ../lib/libbufio.so
	gcc -o $@ -L../lib bufcat.o -Wl,-rpath=../lib -lbufio

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

bufcat.o: bufcat.c
	gcc -std=c99 -o $@ -c -Wall -I../lib $<
