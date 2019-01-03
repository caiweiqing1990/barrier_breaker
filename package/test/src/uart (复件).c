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
#include "uart.h"

//用法：只要指定serial_t的baud就可以了static 
struct serial_t __seri_conf[] = {
    [0] = {//connect with b board, ttyS0
        .device = "/dev/ttyS0",
        .baud = 38400,
        .databit = 8,
        .parity = 'N',
        .stopbit = 1,
    },
    [1] = {//connect with b board, ttyS0
        .device = "/dev/ttyS1",
        .baud = 76800,
        .databit = 8,
        .parity = 'N',
        .stopbit = 1,
    },
};

int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
        B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300,
        38400,  19200,  9600, 4800, 2400, 1200,  300, };

void set_speed(int fd, int speed)
{
  int   i;
  int   status;
  struct termios   Opt;
  tcgetattr(fd, &Opt);
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
   {
    if  (speed == name_arr[i])
    {
        tcflush(fd, TCIOFLUSH);
        cfsetispeed(&Opt, speed_arr[i]);
        cfsetospeed(&Opt, speed_arr[i]);
        status = tcsetattr(fd, TCSANOW, &Opt);
        if  (status != 0)
            perror("tcsetattr fd1");
        return;
        }
   tcflush(fd,TCIOFLUSH);
   }
}
 
//设置为特诉波特率，比如28800
static int serial_set_speci_baud(struct serial_t *tty,int baud)
{
    struct serial_struct ss,ss_set;
    tcgetattr(tty->fd,&tty->options);
 if (baud==38400){
    cfsetispeed(&tty->options,B38400);
    cfsetospeed(&tty->options,B38400);
    printf("BAUD: success set baud to %d\n",baud);
    }
 else{
    cfsetispeed(&tty->options,B57600);
    cfsetospeed(&tty->options,B57600);
    printf("BAUD: success set baud to %d\n",baud);
    }
    tcflush(tty->fd,TCIFLUSH);/*handle unrecevie char*/
    tcsetattr(tty->fd,TCSANOW,&tty->options);
    if((ioctl(tty->fd,TIOCGSERIAL,&ss))<0){
        printf("BAUD: error to get the serial_struct info:%s\n",strerror(errno));
        return -1;
    }
    ss.flags = ASYNC_SPD_CUST;
    ss.custom_divisor = ss.baud_base / baud;
    printf("ss.custom_divisor = %d \r\n",ss.custom_divisor);
    if((ioctl(tty->fd,TIOCSSERIAL,&ss))<0){
        printf("BAUD: error to set serial_struct:%s\n",strerror(errno));
        //return -2;
    }
    ioctl(tty->fd,TIOCGSERIAL,&ss_set);
    printf("custom_divisor=%d,baud_base=%d\n",
            ss_set.custom_divisor,ss_set.baud_base);
    return 0;
}

/*get serial's current attribute*/

static int serial_get_attr(struct serial_t *tty)
{
    if(tcgetattr(tty->fd,&tty->options) != 0){
        printf("SERIAL: can't get serial's attribute\n");
        return -1;
    }
    return 0;
}
/*update serial's attrbute*/
static int serial_attr_update(struct serial_t *tty)
{
    tcflush(tty->fd,TCIFLUSH);/*handle unrecevie char*/
    if((tcsetattr(tty->fd,TCSANOW,&tty->options)) < 0){
        return -1;
    }
    return 0;
}
static int serial_init_databit(struct serial_t *tty)
{
    if(serial_get_attr(tty)<0)
        return -1;
    tty->options.c_cflag &= ~CSIZE;
    switch(tty->databit){
        case 5: tty->options.c_cflag |= CS5;break;
        case 6: tty->options.c_cflag |= CS6;break;
        case 7: tty->options.c_cflag |= CS7;break;
        case 8: tty->options.c_cflag |= CS8;break;
        default:
            printf("SERIAL: unsupported databit %d\n",tty->databit);
            return -2;
}
    if(serial_attr_update(tty) < 0)
        return -3;
    printf("SERIAL: set databit to %d\n",tty->databit);
    return 0;
}
static int serial_init_parity(struct serial_t *tty)
{
    if(serial_get_attr(tty)<0)
        return -1;
    /*ignore framing and parity error*/
    tty->options.c_iflag = IGNPAR;
    switch (tty->parity){
        case 'n':
        case 'N':
            /* Clear parity enable */
            tty->options.c_cflag &= ~PARENB;
            /* Enable parity checking */
            tty->options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            /* 设置为奇校检*/
            tty->options.c_cflag |= (PARODD|PARENB);
            /* Disnable parity checking */
            tty->options.c_iflag |= (INPCK|ISTRIP);
            break;
        case 'e':
        case 'E':
            /* Enable parity */
            tty->options.c_cflag |= PARENB;
            /* 转换为偶效验*/
            tty->options.c_cflag &= ~PARODD;
            /* Disnable parity checking */
            tty->options.c_iflag |= (INPCK|ISTRIP);
            break;
        default:
            printf("SERIAL: unsupported parity %c\n",tty->parity);
            return -2;
}
    if(serial_attr_update(tty) < 0)
        return -3;
    printf("SERIAL: set parity to %c\n",tty->parity);
    return 0;
}
static int serial_init_stopbit(struct serial_t *tty)
{
    if(serial_get_attr(tty)<0)
        return -1;
    switch(tty->stopbit){
        case 1:
            tty->options.c_cflag &= ~CSTOPB;break;
        case 2:
            tty->options.c_cflag |= CSTOPB;break;
        default:
            printf("SERIAL: unsupported stopbit %d\n",tty->stopbit);
            return -2;
    }
    if(serial_attr_update(tty) < 0)
        return -3;
    printf("SERIAL: set stopbit to %d\n",tty->stopbit);
    return 0;
}



/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄*
*@param  databits 类型  int 数据位   取值 为 7 或者8*
*@param  stopbits 类型  int 停止位   取值为 1 或者2*
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
 if  ( tcgetattr( fd,&options)  !=  0)
  {
    perror("SetupSerial 1");
    return(FALSE);
  }
  options.c_cflag &= ~CSIZE;
  // 激活选项有CLOCAL和CREAD，用于本地连接和接收使能
  options.c_cflag |= (CLOCAL | CREAD);        //一般必设置的标志
  switch (databits) /*设置数据位数*/
  {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr,"Unsupported data size\n");
        return (FALSE);
    }
  switch (parity)
    {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;   /* Clear parity enable */
        options.c_iflag &= ~INPCK;     /* Enable parity checking */
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
        options.c_iflag |= INPCK;             /* Disnable parity checking */
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB;     /* Enable parity */
        options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
        options.c_iflag |= INPCK;       /* Disnable parity checking */
        break;
    case 'S':
    case 's':  /*as no parity*/
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_iflag |= INPCK;            //disable pairty checking
        break;
    default:
        fprintf(stderr,"Unsupported parity\n");
        return (FALSE);
        }
  /* 设置停止位*/
  switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr,"Unsupported stop bits\n");
        return (FALSE);
    }
  /* Set input parity option */
  if (parity != 'n')
        options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 0; // 15 seconds
    options.c_cc[VMIN] = 0;
  tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
  if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
  return (TRUE);
}

int open_uart(void)
{
    int fd;
    fd = open(__seri_conf[0].device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd>0){
    __seri_conf->fd = fd;
    printf("open tty device:%s , fd=%d\n",__seri_conf->device, __seri_conf->fd);
    }
   //set_speed(__seri_conf->fd,38400);
    serial_set_speci_baud(__seri_conf, __seri_conf[0].baud);
    if(serial_init_databit(__seri_conf)<0)
        printf("serial_init_databit error\n");
    if(serial_init_parity(__seri_conf)<0)
        printf("serial_init_parity error\n");
    if(serial_init_stopbit(__seri_conf)<0)
        printf("serial_init_stopbit error\n");
    //struct termios opt;
    tcgetattr(__seri_conf->fd,&__seri_conf->options);
    // 激活选项有CLOCAL和CREAD，用于本地连接和接收使能
    __seri_conf->options.c_cflag |= (CLOCAL | CREAD);        //一般必设置的标志
    __seri_conf->options.c_iflag &=~(BRKINT|ICRNL|INPCK|ISTRIP|IXON|INLCR|IGNCR);
    __seri_conf->options.c_lflag &=~(ICANON|ECHO|ECHOE|ISIG);
    __seri_conf->options.c_oflag &=~(OPOST|ONLCR | OCRNL);
    __seri_conf->options.c_cc[VTIME] = 0; // 15 seconds
    __seri_conf->options.c_cc[VMIN] = 0;
    if(tcsetattr(__seri_conf->fd,TCSANOW,&__seri_conf->options)!=0)
        printf("error");
    return 0;
}

