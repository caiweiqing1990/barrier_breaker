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
#include "sp2349.h"
#include "uart.h"
int fd_sp2349;
extern struct serial_t __seri_conf[];
unsigned char bUartBusy=0;		//上位机串口发送状态标志,"1"表示上位机串口"忙"(正发送数据
							//到子串口,新数据只需送入子串口发送缓冲区即可,"0"表示上位
							//机串口"空闲",需用户程序将"TI=1"来启动子串口数据发送

unsigned char bInstructFlag=0;	//上位机发送指令请求标志,上位机需要发送指令时必须
							//先将待发发送指令放到"InstructBuf",之后将该
							//标志设置为"1"即可.标志变为"0"后可继续发送指令

unsigned char bUart0RFlag,bUart1RFlag,bUart2RFlag;	//上位机接收到子串口数据
							//标志,"1"表示上位机接收到相应子串口数据,主程序可通过查询
							//该标志的方式判断相应子串口是否接收到新数据(主程序取走刚
							//收到的新数据后,必须将该标志清"0",以便识别下一个新数据)

char Uart0TBuf[5]; 	//子串口0数据发送缓冲区(大小可由用户随意配置)
char Uart1TBuf[8]; 	//子串口1数据发送缓冲区(大小可由用户随意配置)
char Uart2TBuf[8]; 	//子串口2数据发送缓冲区(大小可由用户随意配置)

unsigned char Uart0RBuf,Uart1RBuf,Uart2RBuf;	//上位机接收子串口数据缓
							//冲存储器,上位机通过判断"bUart0RFlag"等标志
							//是否为"1",来决定是否读取该寄存器的数据

unsigned char UartPointer=0;	//子串口发送数据指针,中断服务程序通过该指针轮询
							//发送数据到各子串口,保证各子串口的数据不丢失

unsigned char InstructBuf;	//指令缓冲(用户可将设置SP2339/SP2349的指令放入
							//该寄存器,由驱动程序自动完成指令发送)

unsigned char Uart0TNum,Uart1TNum,Uart2TNum;	//等待发送到各
							//子串口的字节数,"0"表示相应子串口发送缓冲区"空"
							//主程序可以通过判断该标志是否为"0"来决定是否向
							//相应子串口发送后续数据

//如果对应子串口波特率为默认值,则不用定义对应子串口的分频系数变量
unsigned char Uart0Coef,Uart1Coef,Uart2Coef;	//子串口分频系数/波特率控制寄存
							//器,具体用法请参见设置子串口的波特率指令部分

//如果对应子串口波特率为默认值,则不用定义对应子串口的分频系数变量
unsigned char Uart0CoefBuf,Uart1CoefBuf,Uart2CoefBuf;	//分频系数缓冲寄存器

void switch_uart_port(unsigned int cmd)
{
	switch(cmd)
	{
		case RS232_EN:	// ADRI 0 ADRI 1 两位地址都置为 0 子串口0
			ioctl(fd_sp2349, 8, 0);
			printf("uart0_SP1!\n");
			break;
		case IRIDIUM_EN:	// ADRI 0 ADRI 1 两位地址都置为  子串口1
			ioctl(fd_sp2349, 9, 1);
			printf("uart1_SP1!\n");
			break;
		case GPS_EN:	// ADRI 0 ADRI 1 两位地址都置为  子串口2
			ioctl(fd_sp2349, 10, 1);
			printf("uart2_SP1!\n");
			break;
		case MOTHER_SP1:	// ADRI 0 ADRI 1 两位地址都置为 1 母串口
			ioctl(fd_sp2349, 11, 1);
			printf("MOTHER_SP1!\n");
			break;
		default:
			break;
	}
}

char null[]={0x00};
int Uart0_num=0,Uart1_num=0,Uart2_num=0;
int serial_interrupt()  {
	//switch_uart_port(MOTHER_SP1);			
//T_Instruction:	*uart0, char *uart1, char *uart2char *buf	
		printf("UartPointer=%d\n",UartPointer);			//已经产生发送中断,准备解析下一个需要发送的数据
		switch(UartPointer){	//判断即将向哪个子串口发送数据?
			case 0:{			//准备向子串口0发送数据
				UartPointer++;	//下一次发送中断准备向子串口1发送数据
//				if((--Uart0CoefBuf)==0){	//由于子串口0的波特率为9600bps,
//					Uart0CoefBuf=Uart0Coef;	//所以程序中不用定义和使用这两个变量
					if(Uart0TNum){			//串口0发送缓存区是否有数据需要发送?
						switch_uart_port(RS232_EN);			//有数据需要发送,则选通子串口0的地址
						write(__seri_conf->fd,Uart0TBuf+Uart0_num,1);
						//SBUF=Uart0TBuf[--Uart0TNum];//将串口0发送缓存中的一个字节
						--Uart0TNum;
						Uart0_num++;
						serial_interrupt();
						printf(">>Uart0TNum<<=%d\n",Uart0TNum);
						//goto T_Instruction;
						//break;						//由子串口0发送到下位机
					}
					else goto T_NopInstruction;		//若子串口0没有数据发送则准备发送空指令
//				}
//				else goto T_NopInstruction;
			}

			case 1:{
				//switch_uart_port(RS232_EN);		//准备向子串口1发送数据
				UartPointer++;	//下一次发送中断准备向串口2发送数据
//				if((--Uart1CoefBuf)==0){	//由于子串口1的波特率为4800,需要进行二分
//					Uart1CoefBuf=Uart1Coef;	//频时序处理,只有时序到达后才可发送数据
					if(Uart1TNum){			//串口1发送缓存区是否有数据需要发送?
						switch_uart_port(IRIDIUM_EN);
						//AdrOut0=1;			//有数据需要发送,则选通子串口1的地址
						//AdrOut1=0;
						write(__seri_conf->fd,Uart1TBuf+Uart1_num,1);
						//SBUF=Uart1TBuf[--Uart1TNum];//将串口1发送缓存中的一个字节
						--Uart1TNum;
						Uart1_num++;
						serial_interrupt();
											//由子串口1发送到下位机
						//goto T_Instruction;//break;	
					}
					else goto T_NopInstruction;		//若子串口1没有数据发送则准备发送空指令
//				}
//				else goto T_NopInstruction; //若分频时间没有到则不能向子串口1发送数据
			}								//而只能够发送一条空指令用于时序配合

			case 2:{
				//switch_uart_port(IRIDIUM_EN);			//准备向子串口2发送数据
				UartPointer++;	//下一次发送中断准备发送空指令
//				if((--Uart2CoefBuf)==0){	//由于子串口2的波特率为2400,需要进行四分
//					Uart2CoefBuf=Uart2Coef;	//频时序处理,只有时序到达后才可发送数据
					if(Uart2TNum){			//串口2发送缓存区是否有数据需要发送?
						switch_uart_port(GPS_EN);
						//AdrOut0=0;			//有数据需要发送,则选通子串口2的地址
						//AdrOut1=1;
						write(__seri_conf->fd,Uart2TBuf+Uart2_num,1);
						//SBUF=Uart2TBuf[--Uart2TNum];//将串口2发送缓存中的一个字节
						--Uart2TNum;
						Uart2_num++;
						serial_interrupt();
						//						//由子串口2发送到下位机
						//goto T_Instruction;//break;
					}
					else goto T_NopInstruction;		//若子串口2没有数据发送则准备发送空指令
//				}
//				else goto T_NopInstruction; //若分频时间没有到则不能向子串口2发送数据
			}								//而只能够发送一条空指令用于时序配合

			default:{
				//switch_uart_port(GPS_EN);
				UartPointer=0;			//下一次发送中断准备向串口0发送数据
T_NopInstruction:
				switch_uart_port(MOTHER_SP1);
				//AdrOut0=1;				//选通目串口准备发送空指令或者其他指令
				//AdrOut1=1;
				printf("bInstructFlag=%d\n",bInstructFlag);
				if(bInstructFlag){		//判断是否需要发送其他指令?
					bInstructFlag=0;	//清除指令发送标志
					//write(__seri_conf->fd,InstructBuf,1);
					printf("----bInstructFlag=%d,%d\n",bInstructFlag,InstructBuf);
					//SBUF=InstructBuf;	//将需要发送的指令发送到串口扩展IC
					serial_interrupt();
					//break;
					//goto T_Instruction;
				}
				else if(Uart0TNum||Uart1TNum||Uart2TNum){//判断所有子串口的数据是否
					write(__seri_conf->fd, null, 1);
					//SBUF=0;				//都已经全部发送完毕?若没有完毕则发送空指令
					serial_interrupt();
					//break;
					//goto T_Instruction;
				}
				else {
					bUartBusy=0;
					Uart0_num=0;
					Uart1_num=0;
					Uart2_num=0;
					printf("bUartBusy=%d\n",bUartBusy);break;
				}//若所有子串口数据都已经发送完毕,则立即break;
			}	//清除"串口忙"标志(主程序将根据该标志来决定是否执行"TI=1"这条指令)
		}
	return tcdrain(__seri_conf->fd);
}

/*****************************************************************************
 功能:上位机发送指令到SP2339/SP2349,该函数主要完成将命令字放入指令缓冲区,并置
      相应标志即可,指令发送将由中断服务函数自动完成
 输入:欲操作的命令字
 输出:无
*****************************************************************************/
void InstructSet (unsigned char InstructWord){	//入口参数:操作SP23X9的命令字
	while(bInstructFlag);				//等待操作SP23X9的其它命令结束
	InstructBuf=InstructWord;			//加载命令字
	bInstructFlag=1;					//置发送命令请求标志
	if(bUartBusy==0){			//判断中断服务程序中的发送部分是否处于激活状态,
//		serial_interrupt();
		bUartBusy=1;			//否则将主动激活上位机中断服务程序的数据发送程序
	}
}

/*****************************************************************************
 功能:初始化上位机串口波特率为38400BPS(采用TIMER 2),子串口1波特率为：9600;
 子串口1波特率为：4800;子串口2波特率为：2400;
 输入:无
 输出:无
*****************************************************************************/
void init_serialcomm(void){
/*	Uart0TNum=8;			//初始化串口0发送缓冲区"空"
	Uart1TNum=8;			//初始化串口1发送缓冲区"空"
	Uart2TNum=8;			//初始化串口2发送缓冲区"空"
	
//	Uart0Coef=0x01;			//由于子串口0的波特率为9600bps,所以
//	Uart0CoefBuf=0x01;		//程序中不用定义和使用这两个变量

	InstructSet(0x1F);		//设置子串口1的波特率为4800BPS
	Uart1Coef=0x02;			//置子串口1的分频系数为"2"
	Uart1CoefBuf=0x01;		//分频系数临时缓存,初始化为"1"

	InstructSet(0x25);		//设置子串口2的波特率为2400BPS
	printf("uart2_init\n");
	Uart2Coef=0x04;			//置子串口2的分频系数为"4"
	Uart2CoefBuf=0x01;		//分频系数临时缓存,初始化为"1"
	*/
}

int open_sp2349(void)
{
   fd_sp2349 = open("/dev/sp2349",O_RDWR | O_NONBLOCK);
   if(fd_sp2349<0)
     {
  	  perror("open_sp2349_port error");
         return -1;
     }
     return 0;
}

