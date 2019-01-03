# build executable on typing make
all: mytest

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -I. -Iinclude -o $@ $^

mytest: mytest.o
	$(CC) -o $@ $^ -L. -lpthread

clean:
	rm -f *.o mytest
