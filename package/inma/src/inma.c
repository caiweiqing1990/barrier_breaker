#include<stdio.h>
#include<stdlib.h>
#include<termios.h>
#include<string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) 
{
    int i=0;  
    int fd,c=0,num;  
    struct termios oldtio,newtio;  
    char buf[256];  
    printf("start...\n");  
    /*打开PC机的COM1通信端口*/  
    fd=open("/dev/ttyS1",O_RDWR | O_NOCTTY | O_NONBLOCK/*| O_NDELAY*/);  
    if(fd<0)  
    {  
        perror("/dev/ttyS1");  
        exit(1);  
    }  
    printf("open...\n");  
    /*将目前终端机的结构保存至oldtio结构*/  
    tcgetattr(fd,&oldtio);  
    /*清除newtio结构，重新设置通信协议*/  
    bzero(&newtio,sizeof(newtio));  
    /*通信协议设为8N1,8位数据位,N没有效验,1位结束位*/  
    newtio.c_cflag = B38400 |CS8|CLOCAL|CREAD;  
    newtio.c_iflag = IGNPAR;  
    newtio.c_oflag = 0;  
    /*设置为正规模式*/  
    newtio.c_lflag=ICANON;  
    /*清除所有队列在串口的输入*/  
    tcflush(fd,TCIFLUSH);   /*新的termios的结构作为通信端口的参数*/  
    tcsetattr(fd,TCSANOW,&newtio);  
    printf("reading...\n");  
    while(1)  
    {  
        num = read(fd,buf, 256);  
        buf[255] = 0;  
        if(num > 0 && num <= 256)  
        {   
            buf[num]=0;  
            printf("%s", buf);  
            fflush(stdout);  
        }  
    }  
    printf("close...\n");  
    close(fd);  
    /*恢复旧的通信端口参数*/  
    tcsetattr(fd,TCSANOW,&oldtio);
}
