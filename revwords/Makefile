all: revwords

clean:
	rm -f revwords
	rm -f revwords2.c
	$(MAKE) -C ../lib clean

revwords: ../lib/libhelpers.so
	gcc -o $@ revwords.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
