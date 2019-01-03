#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <string.h> 
#include <linux/serial.h>
#include <pthread.h>
#include "uart.h"
#include "sqc.h"  
extern struct serial_t __seri_conf[];

int main(int argc, char **argv) {
	if(open_power()<0)
	return -1;
 	/*if(open_uart_at()<0)
    	return -1;*/
    	if(uart_pthread()<0)
    	return -1;    	
	

  //  close(__seri_conf->fd);
    return 0;
}
