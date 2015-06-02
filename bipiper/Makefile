all: forking

clean:
	rm -f forking
	$(MAKE) -C ../lib clean

forking: ../lib/libbufio.so
	gcc -std=c99 -o $@ forking.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
