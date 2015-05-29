all: bufcat

clean:
	rm -f bufcat
	rm -f err.txt
	rm -f bufcat2.c
	$(MAKE) -C ../lib clean

bufcat: ../lib/libbufio.so
	gcc -o $@ bufcat.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
