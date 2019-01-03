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
#include "power_mode.h"
int fd_power;
int open_power(void){
   fd_power = open("/dev/power_mode",O_RDWR | O_NONBLOCK);
   if(fd_power<0)
     {
  	  perror("open_fd_power_port error");
         return -1;
     }
     return 0;
}





