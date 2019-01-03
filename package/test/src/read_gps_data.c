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
#include "sqc.h" 

int main(int argc, char **argv) {
	
    	if(uart_pthread()<0)
    	return -1;
 
       return 0;
}
