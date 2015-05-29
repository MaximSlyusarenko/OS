all: revwords

clean:
	rm -f revwords.o
	rm -f revwords
	rm -f revwords2.c
	$(MAKE) -C ../lib clean

revwords: revwords.o ../lib/libhelpers.so
	gcc -o $@ -L../lib revwords.o -Wl,-rpath=../lib -lhelpers

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

revwords.o: revwords.c
	gcc -std=c99 -o $@ -c -Wall -I../lib $<
