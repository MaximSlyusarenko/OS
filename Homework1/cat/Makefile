all: cat

clean:
	rm -f cat.o
	rm -f cat

cat: cat.o ../lib/libhelpers.so
	gcc -o $@ -L../lib -lhelpers cat.o ../lib/libhelpers.so

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

cat.o: cat.c
	gcc -o $@ -c -Wall -I../lib $<
