OBJS += read_gps_data.o sqc.o power_mode.o
#mix_test.o socket_client.o sp2349.o uart.o  main_power.o
mix_test: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -pthread  -o  mix_test 

%.o: %.c %.h
	$(CC) $(CFLAGS)  -c -g -o $@ $< $(LDFLAGS)


clean:
	rm -f rbcfg *.o
	
all: mix_test

