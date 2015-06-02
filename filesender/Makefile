all: filesender

clean:
	rm -f filesender
	$(MAKE) -C ../lib clean

filesender: ../lib/libbufio.so
	gcc -std=c99 -o $@ filesender.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
