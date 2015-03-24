all: cat

clean:
	rm -f cat.o
	rm -f cat
	rm -f cat2.c
	$(MAKE) -C ../lib clean

cat: cat.o ../lib/libhelpers.so
	gcc -o $@ -L../lib cat.o ../lib/libhelpers.so -lhelpers

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

cat.o: cat.c
	gcc -o $@ -c -Wall -I../lib $<
