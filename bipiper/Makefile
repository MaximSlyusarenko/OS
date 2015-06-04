all: forking polling

clean:
	rm -f forking
	rm -f polling
	$(MAKE) -C ../lib clean

forking: ../lib/libbufio.so
	gcc -std=c99 -o $@ forking.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

polling: ../lib/libbufio.so
	gcc -std=c99 -o $@ polling.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
