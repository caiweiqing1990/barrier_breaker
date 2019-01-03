#include <pthread.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <sys/time.h>  
#include <string.h>  
#include <termios.h>  
#include <sys/stat.h>  
#include <fcntl.h>
#include <signal.h>
#include <sys/klog.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h> 
#include "sqc.h" 
#include "power_mode.h"

typedef void (*sighandler_t)(int);
int pox_system(const char *cmd_line)
{
   if(NULL == cmd_line){ //尽管system()函数也能处理空指针
    return (-1);
   }
   int status = 0;
   sighandler_t old_handler;
   old_handler = signal(SIGCHLD, SIG_DFL);
   status = system(cmd_line);
   signal(SIGCHLD, old_handler);

   if(status < 0){
    printf("cmd: %s\t error: %s", cmd_line, strerror(errno));
    return (-1);
   }
 
   if(WIFEXITED(status)){
    printf("normal termination, exit status = %d\n", WEXITSTATUS(status)); //取得cmdstring执行结果
   }
   else if(WIFSIGNALED(status)){
    printf("abnormal termination,signal number =%d\n", WTERMSIG(status)); //如果cmdstring被信号中断，取得信号值
   }
   else if(WIFSTOPPED(status)){
    printf("process stopped, signal number =%d\n", WSTOPSIG(status)); //如果cmdstring被信号暂停执行，取得信号值
   }
   return status;
}

void* com_read_gps(void* pstatu)  
{  
    printtid();  
    int i=0;  
    int fd,c=0,num;  
    struct termios oldtio,newtio;  
    char buf[R_BUF_LEN];  
    printf("start...\n");  
    /*打开PC机的COM1通信端口*/  
    fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY | O_NONBLOCK/*| O_NDELAY*/);  
    if(fd<0)  
    {  
        perror(MODEMDEVICE);  
        exit(1);  
    }  
    printf("open...\n");  
    /*将目前终端机的结构保存至oldtio结构*/  
    tcgetattr(fd,&oldtio);  
    /*清除newtio结构，重新设置通信协议*/  
    bzero(&newtio,sizeof(newtio));  
    /*通信协议设为8N1,8位数据位,N没有效验,1位结束位*/  
    newtio.c_cflag = BAUDRATE_GPS |CS8|CLOCAL|CREAD;  
    newtio.c_iflag = IGNPAR;  
    newtio.c_oflag = 0;  
    /*设置为正规模式*/  
    newtio.c_lflag=ICANON;  
    /*清除所有队列在串口的输入*/  
    tcflush(fd,TCIFLUSH);   /*新的termios的结构作为通信端口的参数*/  
    tcsetattr(fd,TCSANOW,&newtio);  
    printf("reading...\n");  
    while(*(int*)pstatu)  
    {  
        num = read(fd,buf, R_BUF_LEN);  
        buf[R_BUF_LEN-1] = 0;  
        if(num > 0 && num <= R_BUF_LEN)  
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
 
void* com_read_gprs(void* pstatu)  
{  
    printtid();  
    int ret = 0;
    int  i=0;  
    int  fd,c=0,num; 
    int  chars_read; 
    char buffer[BUFSIZ];
    struct termios oldtio,newtio;  
    char buf[R_BUF_LEN];  
    FILE  *gprs_read_fp = popen("ifconfig 3g-wan | grep RUNNING", "r");
    
    printf("start...\n");  
    /*打开PC机的COM1通信端口*/  
    fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY | O_NONBLOCK/*| O_NDELAY*/);  
    if(fd<0)  
    {  
        perror(MODEMDEVICE);  
        exit(1);  
    }  
    printf("open ...\n");  
    /*将目前终端机的结构保存至oldtio结构*/  
    tcgetattr(fd,&oldtio);  
    /*清除newtio结构，重新设置通信协议*/  
    bzero(&newtio,sizeof(newtio));  
    /*通信协议设为8N1,8位数据位,N没有效验,1位结束位*/  
    newtio.c_cflag = BAUDRATE |CS8|CLOCAL|CREAD;  
    newtio.c_iflag = IGNPAR;  
    newtio.c_oflag = 0;  
    /*设置为正规模式*/  
    newtio.c_lflag=ICANON;  
    /*清除所有队列在串口的输入*/  
    tcflush(fd,TCIFLUSH);   /*新的termios的结构作为通信端口的参数*/  
    tcsetattr(fd,TCSANOW,&newtio);  
    printf("reading...\n");  
    while(*(int*)pstatu)  
   {  
        num = read(fd,buf, R_BUF_LEN);  
        buf[R_BUF_LEN-1] = 0;  
        if(num > 0 && num <= R_BUF_LEN){   
            buf[num]=0;  
            if(buf[CSQ_tenths]>=CSQ_demarcation){
            printf("Signal strength is ok\n");
            printf("read char:%s \n", buf);  
            fflush(stdout);
            if ( gprs_read_fp != NULL){
	 	chars_read = fread(buffer, sizeof(char), BUFSIZ-1, gprs_read_fp);
            if (chars_read > 0) {
            	printf( "3g-wan NetStat link \n");
             }//chars_read
            else{
            	printf( "3g-wan crea link \n");
            	//ioctl(fd_power, gprs_on);
            	printf("wait 10 sec Is switched on.....\n");  
            	sleep(10);
		ret = pox_system(cmd_3732_string);
              if(0 != ret)
		{
   	 	   syslog(LOG_USER|LOG_DEBUG,"By ifup wan monitoring the error\n");
		}
            }//else
	     pclose(gprs_read_fp);
          }//gprs_read_fp != NULL
	else{
	 	printf( "To build the query fails \n"); 
	   } //else
         } //buf[6]>='5'
       } //num > 0 && num <= R_BUF_LEN 
   } //while 
    printf("close...\n");  
    close(fd);  
    /*恢复旧的通信端口参数*/  
    tcsetattr(fd,TCSANOW,&oldtio);  
}  
void* com_send_gprs(void* p)  
{  
    printtid();  
    int fd,c=0;  
    struct termios oldtio,newtio;  
    char ch;  
    static char s1[20];  
    printf("Start...\n ");  
    fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY);  
    if(fd<0)  
    {  
        perror(MODEMDEVICE);  
        exit(1);  
    }  
    printf(" Open...\n ");  
     /*将目前终端机的结构保存至oldtio结构*/  
       tcgetattr(fd,&oldtio);  
    /*清除newtio结构，重新设置通信协议*/  
    bzero(&newtio,sizeof(newtio));  
    /*通信协议设为8N1*/  
    newtio.c_cflag =BAUDRATE |CS8|CLOCAL|CREAD; //波特率 8个数据位 本地连接 接受使能  
    newtio.c_iflag=IGNPAR;                      //忽略奇偶校验错误  
    newtio.c_oflag=0;  
    /*设置为正规模式*/  
    newtio.c_lflag=ICANON;                     //规范输入  
    /*清除所有队列在串口的输出*/  
    tcflush(fd,TCOFLUSH);  
    /*新的termios的结构作为通信端口的参数*/  
    tcsetattr(fd,TCSANOW,&newtio);  
    printf("Writing...\n ");  
    ///*  
    while(*(char*)p != 0)  
    {  
        int res = 0;  
        res = write(fd,(char*)p, 1);  
        if(res != 1) printf("send %c error\n", *(char*)p);  
        else printf("send %c ok\n", *(char*)p);  
        ++p;  
    }  
    printf("Close...\n");  
    close(fd);  
    /*还原旧的通信端口参数*/  
    tcsetattr(fd,TCSANOW,&oldtio);  
    printf("leave send thread\n");  
}  
/* 
    开始线程 
    thread_fun  线程函数 
    pthread     线程函数所在pthread变量 
    par     线程函数参数 
    COM_STATU   线程函数状态控制变量 1:运行 0:退出 
*/  
int start_thread_func(void*(*func)(void*), pthread_t* pthread, void* par, int* COM_STATU)  
{  
    *COM_STATU = 1;  
    memset(pthread, 0, sizeof(pthread_t));  
    int temp;  
        /*创建线程*/  
        if((temp = pthread_create(pthread, NULL, func, par)) != 0)  
        printf("Thread creation failed!\n");  
        else  
    {  
        int id = pthread_self();  
                printf("A thread = %u is created\n", *pthread);  
    }  
    return temp;  
}  
/* 
    结束线程 
    pthread     线程函数所在pthread变量 
    COM_STATU   线程函数状态控制变量 1:运行 0:退出 
*/  
int stop_thread_func(pthread_t* pthread, int* COM_STATU)  
{  
    printf("prepare stop thread %u\n", *pthread);  
    *COM_STATU = 0;  
    if(*pthread !=0)   
    {  
                pthread_join(*pthread, NULL);  
    }  
    printf("Thread= %d to exit !\n", *COM_STATU);  
}  
void printtid(void)  
{  
    int id = pthread_self();  
    printf("in thread %u\n", id);  
}  
int uart_pthread(void)  
{  
    pthread_t thread[3];  
    printtid();  
    const int READ_THREAD_GPRS_ID = 0;  
    const int SEND_THREAD_GPRS_ID = 1;  
    const int READ_THREAD_GPS_ID = 2;
    int COM_READ_STATU = 0;  
    int COM_SEND_STATU = 0; 
    ioctl(fd_power, gps_on);
    printf("wait 300 sec\n");
#if GPS
    if(start_thread_func(com_read_gps, &thread[READ_THREAD_GPS_ID],  &COM_READ_STATU, &COM_READ_STATU) != 0)  
    {  
        printf("error to leave\n");  
        return -1;  
    }
    sleep(300);
    printf("wake after 30 sec\n");  
#endif

#if GPRS
    if(start_thread_func(com_send_gprs, &thread[SEND_THREAD_GPRS_ID], "AT+CSQ\r\n", &COM_SEND_STATU) != 0)  
    {  
        printf("error to leave\n");  
        return -1;  
    }   
    printf("wait 3 sec\n");  
 //   sleep(1);  
    printf("wake after 3 sec\n");  
    if(start_thread_func(com_read_gprs, &thread[READ_THREAD_GPRS_ID],  &COM_READ_STATU, &COM_READ_STATU) != 0)  
    {  
        printf("error to leave\n");  
        return -1;  
    }  
    printtid();  
    printf("wait 10 sec\n");  
    sleep(3);  
    printf("wake after 10 sec\n");  
    stop_thread_func(&thread[READ_THREAD_GPRS_ID], &COM_READ_STATU);  
    stop_thread_func(&thread[SEND_THREAD_GPRS_ID], &COM_SEND_STATU);
#endif

#if GPS  
    stop_thread_func(&thread[READ_THREAD_GPS_ID], &COM_READ_STATU);  
#endif   
    return 0;  
}  

