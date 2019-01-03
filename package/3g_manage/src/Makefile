# build executable on typing make
all: 3g_manage power_mode

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -I. -Iinclude -o $@ $^

3g_manage: 3g_manage.o
	$(CC) -o $@ $^ -L. -lpthread
	
power_mode: power_mode.o
	$(CC) -o $@ $^ -L. -lpthread
	
clean:
	rm -f *.o 3g_manage power_mode
