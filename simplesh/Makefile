all: simplesh

clean:
	rm -f simplesh
	$(MAKE) -C ../lib clean

simplesh: ../lib/libhelpers.so ../lib/libbufio.so
	gcc -std=c99 -o $@ simplesh.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers -lbufio

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
