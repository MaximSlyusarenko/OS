all: cat

clean:
	rm -f cat
	rm -f cat2.c
	$(MAKE) -C ../lib clean

cat: ../lib/libhelpers.so
	gcc -o $@ cat.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)
