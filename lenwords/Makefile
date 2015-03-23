all: lenwords

clean:
	rm -f lenwords.o
	rm -f lenwords
	$(MAKE) -C ../lib clean

lenwords: lenwords.o ../lib/libhelpers.so
	gcc -o $@ -L../lib -lhelpers lenwords.o ../lib/libhelpers.so

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

lenwords.o: lenwords.c
	gcc -o $@ -c -Wall -I../lib $<
