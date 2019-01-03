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
//#include "uart.h"
//#include "sp2349.h"
#include "power_mode.h"

//extern struct serial_t __seri_conf[];
void print_usage(char *file)
{
	printf("Usage:\n");
	printf("%s <dev> <on|off|switch>\n",file);
	printf("eg. \n");
	printf("%s gprs on\n", file);
	printf("%s gprs off\n", file);
	printf("%s sat_sm2500_reset\n", file);
	printf("%s sat_sm2500_on\n", file);
	printf("%s sat_sm2500_off\n", file);
}

int main(int argc, char **argv) {
	
	if (argc != 3){
		print_usage(argv[0]);
		return 0;
	}
	if(open_power()<0)
	return -1;
	/*if(open_uart()<0)
    	return -1;
    	if(open_sp2349()<0)
    	return -1;*/
	if(!strcmp("gprs", argv[1])){
		if (!strcmp("on", argv[2])){
			ioctl(fd_power, gprs_on);
		}
		else if (!strcmp("off", argv[2])){
			ioctl(fd_power, gprs_off);
		}
		else{
			print_usage(argv[0]);
			return 0;
		}
	}
	else if(!strcmp("sat_sm2500", argv[1])){
		if (!strcmp("reset", argv[2])){
			ioctl(fd_power, sat_system_reset);
		}
		else if (!strcmp("on", argv[2])){
			ioctl(fd_power, sat_on);
		}
		else if (!strcmp("off", argv[2])){
			ioctl(fd_power, sat_off);
		}
		else{
			print_usage(argv[0]);
			return 0;
		}
	}
	else{
		print_usage(argv[0]);
		return 0;
	}
  /* int i;
    int len;
    int nread= 0;      
    char read_buf[256];
    char write_buf[256];
    char no[]={0x0A,0x0B,0x0C,0x0D,0x0E};//电话号码
    char on[]={0x01,0x02,0x03,0x04,0x05};//电话号码
    char n[]={0x09,0x08,0x07,0x06,0x0F};//电话号码
    Uart0TNum=sizeof(no);
    Uart1TNum=sizeof(on);
    Uart2TNum=sizeof(n);
    strncpy(Uart0TBuf, no ,sizeof(no));
    strncpy(Uart1TBuf, on ,sizeof(on));
    strncpy(Uart2TBuf, n ,sizeof(n));
   //  ioctl(fd_power, 1, 0);
 //      init_serialcomm();
 	while(1){
      Uart0TNum=sizeof(no);
      Uart1TNum=sizeof(on);
      Uart2TNum=sizeof(n);
      strncpy(Uart0TBuf, no ,sizeof(no));
      strncpy(Uart1TBuf, on ,sizeof(on));
      strncpy(Uart2TBuf, n ,sizeof(n));
 	if(bUartBusy==0){	//判断发送中断是否处于激活状态,若没有激活则：
	     bUartBusy=1;	//置发送程序已经激活的标志
	     serial_interrupt();
	}
	if(bUartBusy==0){	//判断发送中断是否处于激活状态,若没有激活则：
	    bUartBusy=1;	//置发送程序已经激活的标志
	    serial_interrupt();
	}
	if(bUartBusy==0){	//判断发送中断是否处于激活状态,若没有激活则：
	    bUartBusy=1;
	    serial_interrupt();	//置发送程序已经激活的标志
	}
  } */
   /* while(1)
    {    
        nread= 0;
        len = 0;
        bzero(read_buf, sizeof(read_buf));    //类似于memset
        bzero(write_buf, sizeof(write_buf));
        while( (nread= read(__seri_conf->fd, read_buf, sizeof(read_buf))) > 0 )
        {
         printf("read %s \n", read_buf);
        }
        write_buf[len] = '\0';  
        printf("Len %d \n", len);
        printf("write %s \n", write_buf);
        nread= write(__seri_conf->fd, write_buf, len);
        printf("write %d chars\n",nread);
        printf("main_fd=%d",__seri_conf->fd);
        write(__seri_conf->fd, on, 5);
        sleep(2);
    }
    close(__seri_conf->fd);*/
    return 0;
}
