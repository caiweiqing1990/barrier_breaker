#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>   /* signal() */
#include <fcntl.h> /* file control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h> /* 注意此处：signal.h要先include进来 */
#include <ctype.h>
#include <sys/select.h> /* select() */

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>

int init_serial(int *fd, char *device, int baud_rate)
{
  int fd_serial = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd_serial < 0)
  {
    perror("open fd_serial");
    return -1;
  }

  *fd = fd_serial;

  /* 串口主要设置结构体termios <termios.h> */
  struct termios options;

  /* 1.tcgetattr()用于获取与终端相关的参数
   *   参数fd为终端的文件描述符，返回的结果保存在termios结构体中
   */
  tcgetattr(fd_serial, &options);

  /* 2.修改获得的参数 */
  options.c_cflag |= CLOCAL | CREAD; /* 设置控制模块状态：本地连接，接收使能 */
  options.c_cflag &= ~CSIZE;         /* 字符长度，设置数据位之前，一定要屏蔽这一位 */
  options.c_cflag &= ~CRTSCTS;       /* 无硬件流控 */
  options.c_cflag |= CS8;            /* 8位数据长度 */
  options.c_cflag &= ~CSTOPB;        /* 1位停止位 */
  options.c_iflag |= IGNPAR;         /* 无奇偶校验 */
  options.c_oflag = 0;               /* 输出模式 */
  options.c_lflag = 0;               /* 不激活终端模式 */
  cfsetospeed(&options, baud_rate);    /* 设置波特率 */

  /* 3.设置新属性: TCSANOW，所有改变立即生效 */
  tcflush(fd_serial, TCIFLUSH);      /* 溢出数据可以接收，但不读 */
  tcsetattr(fd_serial, TCSANOW, &options);

  return 0;
}

int satfd = 0;

int flag = 0;

static void *func_xx(void *p)
{
  fd_set fds;
  FD_SET(satfd,&fds);
  char data[1024]={0};
  int  idx=0;
  int  n,i,flg=0,ncnt=0;
  
  while(1)
  {
      switch(select(satfd+1, &fds, NULL, NULL, NULL))
      {
        case -1:break;
        case  0:break;
        default:
			if (FD_ISSET(satfd, &fds))
			{
				while(1)
				{
				  if(idx==0)
				  {
					memset(data,0,1024);
					flg=0;
					ncnt=0;
				  }
				  n = read(satfd, &data[idx], 1);
				  
				  if(n>0)
				  {
				  	printf("%s", data);
					if(data[idx]=='\r') data[idx]='\n';

					if(data[idx]=='\n') ncnt++;
					else ncnt = 0;

					if(ncnt==4)
					{
					  idx=2;
					  flg=0;
					  ncnt=0;
					  continue;
					}

					idx++;

					if(idx==1 && data[0] != '\n')
					{
					  idx = 0;
					  continue;
					}
					else if(idx==2 && data[1] != '\n')
					{
					  idx = 0;
					  continue;
					}
					else if(idx==3 && data[2] == '\n')
					{
					  idx = 2;
					  continue;
					}
					else if(idx>4)
					{
					  
					  if(strstr(data,"\n\nOK\n\n"))
					  {
						//printf("data OK\n");
						//flag = 1;
					  }
					  else if(strstr(data,"+CLCC"))
					  {
						printf("CLCC\n");
					  }

					}
				  }
				  else
				  {
					break;
				  }
				}
			}
      }
  }
}

int uart_send(int fd, char *data, int datalen)
{
  int len = 0;
  len = write(fd, data, datalen); /* 实际写入的长度 */
  if (len == datalen)
  {
    return len;
  }
  else
  {
    tcflush(fd, TCOFLUSH); /* TCOFLUSH，刷新写入的数据，但不传送 */
    return -1;
  }

  return 0;
}

int myexec(char *command, char *result, int *maxline)
{
  FILE *pp = popen(command, "r");
  if(NULL == pp) return -1;
  //satfi_log("execute shell : %s\n", command);
  char tmp[1024];
  if(result!=NULL)
  {
    int line=0;
    while(fgets(tmp,sizeof(tmp),pp)!=NULL)
    {
      if('\n' == tmp[strlen(tmp)-1]) tmp[strlen(tmp)-1] = '\0';
      if(maxline!=NULL && line++<*maxline) strcat(result,tmp);
      //satfi_log("%s\n", tmp);
    }
    *maxline = line;
  }
  int rc = pclose(pp);
  return rc;
}

int myping(void)
{
	char tmpBuf[128] = {0};
	int maxline = 2;
	myexec("ping -c 1 -W 20 -s 0 -I 3g-wan8 47.89.48.61", tmpBuf, &maxline);
	
	if(strstr(tmpBuf,"8 bytes from") == NULL)
	{
		printf("func_y:ping no TSC pass\n");
		return 1;
	}	
	else
	{
		printf("func_y:ping TSC pass\n");
		return 0;
	}
}

static void set_block(int socket,int on) {
    int flags;
    flags = fcntl(socket,F_GETFL,0);
    if (on==0) {
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    }else{
        flags &= ~ O_NONBLOCK;
        fcntl(socket, F_SETFL, flags);
    }
}

int connecttsc(char* ip, int port)
{
	int fd;
	struct sockaddr_in addr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	set_block(fd, 0);
	
	connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	
	fd_set          fdwrite;
	struct timeval  tvSelect;
	FD_ZERO(&fdwrite);
	FD_SET(fd, &fdwrite);
	tvSelect.tv_sec = 10;
	tvSelect.tv_usec = 0;
	int retval = select(fd + 1,NULL, &fdwrite, NULL, &tvSelect);
	if (retval<0) {
		close(fd);
		return 1;
	}else if(retval==0){
		perror("connect timeout\n");
		close(fd);
		return 1;
	}else{	
		printf("connect success\n");
		return 0;
	}
}

void audiosend(char* filename)
{
    int sockfd;   
    char buffer[1024];   
    struct sockaddr_in server_addr;   
    int portnumber, nbytes;  
    if (inet_aton("127.0.0.1", &server_addr.sin_addr) == 0) {   
        fprintf(stderr, "the hostip is not right!");   
        exit(1);   
    }

    // 创建套接字   
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {   
        fprintf(stderr, "Socket Error:%s\n\a", strerror(errno));   
        exit(1);   
    }   
    // 填充服务器的地址信息   
    server_addr.sin_family = AF_INET;   
    server_addr.sin_port = htons(12060);   
    // 向服务器发起连接   
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {   
        fprintf(stderr, "Connect Error:%s\n\a", strerror(errno));   
        exit(1);   
    }   

	int pcmfd = open(filename, O_RDONLY);
    if (pcmfd < 0) 
	{
        printf("open of wav failed\n");
        //exit(1);
    }
	
    int dspfd;
    int arg;   /* ioctl arg */
    int ret;   /* return value */
	int rate = 8000;
	int bits = 16;
	int channels = 1;
	
    unsigned char buff[rate * bits * channels / 8]; //buff里面正好放一秒钟的音频
    /* open device */
    //dspfd = open("/dev/dsp", O_WRONLY);
    dspfd = open("/dev/dsp", O_RDONLY);
    if (dspfd < 0) 
	{
        printf("open of /dev/dsp failed\n");
        exit(1);
    }

    /* set bits */
    arg = bits;
    ret = ioctl(dspfd, SOUND_PCM_WRITE_BITS, &arg);
    if (ret == -1)
    {
		perror("SOUND_PCM_WRITE_BITS ioctl failed");
	}

    /* set channels */
    arg = channels;
    ret = ioctl(dspfd, SOUND_PCM_WRITE_CHANNELS, &arg);
    if (ret == -1)
    {
        perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
	}

    /* set rate */
    arg = rate;
    ret = ioctl(dspfd, SOUND_PCM_WRITE_RATE, &arg);
    if (ret == -1)
    {
		perror("SOUND_PCM_WRITE_WRITE ioctl failed");
	} 
	
	int fdread;
	if(pcmfd > 0)
	{
		fdread = pcmfd;
	}
	else
	{
		fdread = dspfd;
	}
	
    while (1) 
	{
		bzero(buff,sizeof(buff));
		if((ret = read(fdread, buff, sizeof(buff))) <= 0)
		{
			break;
		}
		printf("%d\n",ret);

		if (write(sockfd, buff, ret) == -1) {   
			fprintf(stderr, "Write error:%s\n", strerror(errno));   
			exit(1);   
		}
		if(pcmfd > 0)
		{
			sleep(1);
		}
    }

    close(sockfd);
	close(dspfd);
}

/* 串口接收数据 */
int main(int argc, char *argv[])
{
	printf("mytest /dev/ttySAT0 115200 ATD13112121509\;");
	init_serial(&satfd, argv[1], argv[2]);
	printf("satfd = %d\n",satfd);
	sleep(1);
	pthread_t thread_checksat;
	pthread_create(&thread_checksat, NULL, func_xx, NULL);
	char buf[1024] = {0};

	sprintf(buf,"%s\r\n", argv[3]);
	printf("buf:%s",buf);
	uart_send(satfd, buf, strlen(buf));	
	while(1)
	{		
		sleep(1);
	}
	return 0;
}
