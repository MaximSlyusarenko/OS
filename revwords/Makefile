all: revwords

clean:
	rm -f revwords.o
	rm -f revwords
	rm -f revwords2.c
	$(MAKE) -C ../lib clean

revwords: revwords.o ../lib/libhelpers.so
	gcc -o $@ -L../lib -lhelpers revwords.o ../lib/libhelpers.so

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

revwords.o: revwords.c
	gcc -o $@ -c -Wall -I../lib $<
