OBJS += inma.o
#mix_test.o socket_client.o sp2349.o uart.o  main_power.o
inma: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -pthread  -o  inma

%.o: %.c %.h
	$(CC) $(CFLAGS)  -c -g -o $@ $< $(LDFLAGS)


clean:
	rm -f rbcfg *.o
	
all: inma

