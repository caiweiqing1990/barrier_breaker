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
unsigned char bUartBusy=0;		//��λ�����ڷ���״̬��־,"1"��ʾ��λ������"æ"(����������
							//���Ӵ���,������ֻ�������Ӵ��ڷ��ͻ���������,"0"��ʾ��λ
							//������"����",���û�����"TI=1"�������Ӵ������ݷ���

unsigned char bInstructFlag=0;	//��λ������ָ�������־,��λ����Ҫ����ָ��ʱ����
							//�Ƚ���������ָ��ŵ�"InstructBuf",֮�󽫸�
							//��־����Ϊ"1"����.��־��Ϊ"0"��ɼ�������ָ��

unsigned char bUart0RFlag,bUart1RFlag,bUart2RFlag;	//��λ�����յ��Ӵ�������
							//��־,"1"��ʾ��λ�����յ���Ӧ�Ӵ�������,�������ͨ����ѯ
							//�ñ�־�ķ�ʽ�ж���Ӧ�Ӵ����Ƿ���յ�������(������ȡ�߸�
							//�յ��������ݺ�,���뽫�ñ�־��"0",�Ա�ʶ����һ��������)

char Uart0TBuf[5]; 	//�Ӵ���0���ݷ��ͻ�����(��С�����û���������)
char Uart1TBuf[8]; 	//�Ӵ���1���ݷ��ͻ�����(��С�����û���������)
char Uart2TBuf[8]; 	//�Ӵ���2���ݷ��ͻ�����(��С�����û���������)

unsigned char Uart0RBuf,Uart1RBuf,Uart2RBuf;	//��λ�������Ӵ������ݻ�
							//��洢��,��λ��ͨ���ж�"bUart0RFlag"�ȱ�־
							//�Ƿ�Ϊ"1",�������Ƿ��ȡ�üĴ���������

unsigned char UartPointer=0;	//�Ӵ��ڷ�������ָ��,�жϷ������ͨ����ָ����ѯ
							//�������ݵ����Ӵ���,��֤���Ӵ��ڵ����ݲ���ʧ

unsigned char InstructBuf;	//ָ���(�û��ɽ�����SP2339/SP2349��ָ�����
							//�üĴ���,�����������Զ����ָ���)

unsigned char Uart0TNum,Uart1TNum,Uart2TNum;	//�ȴ����͵���
							//�Ӵ��ڵ��ֽ���,"0"��ʾ��Ӧ�Ӵ��ڷ��ͻ�����"��"
							//���������ͨ���жϸñ�־�Ƿ�Ϊ"0"�������Ƿ���
							//��Ӧ�Ӵ��ڷ��ͺ�������

//�����Ӧ�Ӵ��ڲ�����ΪĬ��ֵ,���ö����Ӧ�Ӵ��ڵķ�Ƶϵ������
unsigned char Uart0Coef,Uart1Coef,Uart2Coef;	//�Ӵ��ڷ�Ƶϵ��/�����ʿ��ƼĴ�
							//��,�����÷���μ������Ӵ��ڵĲ�����ָ���

//�����Ӧ�Ӵ��ڲ�����ΪĬ��ֵ,���ö����Ӧ�Ӵ��ڵķ�Ƶϵ������
unsigned char Uart0CoefBuf,Uart1CoefBuf,Uart2CoefBuf;	//��Ƶϵ������Ĵ���

void switch_uart_port(unsigned int cmd)
{
	switch(cmd)
	{
		case RS232_EN:	// ADRI 0 ADRI 1 ��λ��ַ����Ϊ 0 �Ӵ���0
			ioctl(fd_sp2349, 8, 0);
			printf("uart0_SP1!\n");
			break;
		case IRIDIUM_EN:	// ADRI 0 ADRI 1 ��λ��ַ����Ϊ  �Ӵ���1
			ioctl(fd_sp2349, 9, 1);
			printf("uart1_SP1!\n");
			break;
		case GPS_EN:	// ADRI 0 ADRI 1 ��λ��ַ����Ϊ  �Ӵ���2
			ioctl(fd_sp2349, 10, 1);
			printf("uart2_SP1!\n");
			break;
		case MOTHER_SP1:	// ADRI 0 ADRI 1 ��λ��ַ����Ϊ 1 ĸ����
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
		printf("UartPointer=%d\n",UartPointer);			//�Ѿ����������ж�,׼��������һ����Ҫ���͵�����
		switch(UartPointer){	//�жϼ������ĸ��Ӵ��ڷ�������?
			case 0:{			//׼�����Ӵ���0��������
				UartPointer++;	//��һ�η����ж�׼�����Ӵ���1��������
//				if((--Uart0CoefBuf)==0){	//�����Ӵ���0�Ĳ�����Ϊ9600bps,
//					Uart0CoefBuf=Uart0Coef;	//���Գ����в��ö����ʹ������������
					if(Uart0TNum){			//����0���ͻ������Ƿ���������Ҫ����?
						switch_uart_port(RS232_EN);			//��������Ҫ����,��ѡͨ�Ӵ���0�ĵ�ַ
						write(__seri_conf->fd,Uart0TBuf+Uart0_num,1);
						//SBUF=Uart0TBuf[--Uart0TNum];//������0���ͻ����е�һ���ֽ�
						--Uart0TNum;
						Uart0_num++;
						serial_interrupt();
						printf(">>Uart0TNum<<=%d\n",Uart0TNum);
						//goto T_Instruction;
						//break;						//���Ӵ���0���͵���λ��
					}
					else goto T_NopInstruction;		//���Ӵ���0û�����ݷ�����׼�����Ϳ�ָ��
//				}
//				else goto T_NopInstruction;
			}

			case 1:{
				//switch_uart_port(RS232_EN);		//׼�����Ӵ���1��������
				UartPointer++;	//��һ�η����ж�׼���򴮿�2��������
//				if((--Uart1CoefBuf)==0){	//�����Ӵ���1�Ĳ�����Ϊ4800,��Ҫ���ж���
//					Uart1CoefBuf=Uart1Coef;	//Ƶʱ����,ֻ��ʱ�򵽴��ſɷ�������
					if(Uart1TNum){			//����1���ͻ������Ƿ���������Ҫ����?
						switch_uart_port(IRIDIUM_EN);
						//AdrOut0=1;			//��������Ҫ����,��ѡͨ�Ӵ���1�ĵ�ַ
						//AdrOut1=0;
						write(__seri_conf->fd,Uart1TBuf+Uart1_num,1);
						//SBUF=Uart1TBuf[--Uart1TNum];//������1���ͻ����е�һ���ֽ�
						--Uart1TNum;
						Uart1_num++;
						serial_interrupt();
											//���Ӵ���1���͵���λ��
						//goto T_Instruction;//break;	
					}
					else goto T_NopInstruction;		//���Ӵ���1û�����ݷ�����׼�����Ϳ�ָ��
//				}
//				else goto T_NopInstruction; //����Ƶʱ��û�е��������Ӵ���1��������
			}								//��ֻ�ܹ�����һ����ָ������ʱ�����

			case 2:{
				//switch_uart_port(IRIDIUM_EN);			//׼�����Ӵ���2��������
				UartPointer++;	//��һ�η����ж�׼�����Ϳ�ָ��
//				if((--Uart2CoefBuf)==0){	//�����Ӵ���2�Ĳ�����Ϊ2400,��Ҫ�����ķ�
//					Uart2CoefBuf=Uart2Coef;	//Ƶʱ����,ֻ��ʱ�򵽴��ſɷ�������
					if(Uart2TNum){			//����2���ͻ������Ƿ���������Ҫ����?
						switch_uart_port(GPS_EN);
						//AdrOut0=0;			//��������Ҫ����,��ѡͨ�Ӵ���2�ĵ�ַ
						//AdrOut1=1;
						write(__seri_conf->fd,Uart2TBuf+Uart2_num,1);
						//SBUF=Uart2TBuf[--Uart2TNum];//������2���ͻ����е�һ���ֽ�
						--Uart2TNum;
						Uart2_num++;
						serial_interrupt();
						//						//���Ӵ���2���͵���λ��
						//goto T_Instruction;//break;
					}
					else goto T_NopInstruction;		//���Ӵ���2û�����ݷ�����׼�����Ϳ�ָ��
//				}
//				else goto T_NopInstruction; //����Ƶʱ��û�е��������Ӵ���2��������
			}								//��ֻ�ܹ�����һ����ָ������ʱ�����

			default:{
				//switch_uart_port(GPS_EN);
				UartPointer=0;			//��һ�η����ж�׼���򴮿�0��������
T_NopInstruction:
				switch_uart_port(MOTHER_SP1);
				//AdrOut0=1;				//ѡͨĿ����׼�����Ϳ�ָ���������ָ��
				//AdrOut1=1;
				printf("bInstructFlag=%d\n",bInstructFlag);
				if(bInstructFlag){		//�ж��Ƿ���Ҫ��������ָ��?
					bInstructFlag=0;	//���ָ��ͱ�־
					//write(__seri_conf->fd,InstructBuf,1);
					printf("----bInstructFlag=%d,%d\n",bInstructFlag,InstructBuf);
					//SBUF=InstructBuf;	//����Ҫ���͵�ָ��͵�������չIC
					serial_interrupt();
					//break;
					//goto T_Instruction;
				}
				else if(Uart0TNum||Uart1TNum||Uart2TNum){//�ж������Ӵ��ڵ������Ƿ�
					write(__seri_conf->fd, null, 1);
					//SBUF=0;				//���Ѿ�ȫ���������?��û��������Ϳ�ָ��
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
				}//�������Ӵ������ݶ��Ѿ��������,������break;
			}	//���"����æ"��־(�����򽫸��ݸñ�־�������Ƿ�ִ��"TI=1"����ָ��)
		}
	return tcdrain(__seri_conf->fd);
}

/*****************************************************************************
 ����:��λ������ָ�SP2339/SP2349,�ú�����Ҫ��ɽ������ַ���ָ�����,����
      ��Ӧ��־����,ָ��ͽ����жϷ������Զ����
 ����:��������������
 ���:��
*****************************************************************************/
void InstructSet (unsigned char InstructWord){	//��ڲ���:����SP23X9��������
	while(bInstructFlag);				//�ȴ�����SP23X9�������������
	InstructBuf=InstructWord;			//����������
	bInstructFlag=1;					//�÷������������־
	if(bUartBusy==0){			//�ж��жϷ�������еķ��Ͳ����Ƿ��ڼ���״̬,
//		serial_interrupt();
		bUartBusy=1;			//��������������λ���жϷ����������ݷ��ͳ���
	}
}

/*****************************************************************************
 ����:��ʼ����λ�����ڲ�����Ϊ38400BPS(����TIMER 2),�Ӵ���1������Ϊ��9600;
 �Ӵ���1������Ϊ��4800;�Ӵ���2������Ϊ��2400;
 ����:��
 ���:��
*****************************************************************************/
void init_serialcomm(void){
/*	Uart0TNum=8;			//��ʼ������0���ͻ�����"��"
	Uart1TNum=8;			//��ʼ������1���ͻ�����"��"
	Uart2TNum=8;			//��ʼ������2���ͻ�����"��"
	
//	Uart0Coef=0x01;			//�����Ӵ���0�Ĳ�����Ϊ9600bps,����
//	Uart0CoefBuf=0x01;		//�����в��ö����ʹ������������

	InstructSet(0x1F);		//�����Ӵ���1�Ĳ�����Ϊ4800BPS
	Uart1Coef=0x02;			//���Ӵ���1�ķ�Ƶϵ��Ϊ"2"
	Uart1CoefBuf=0x01;		//��Ƶϵ����ʱ����,��ʼ��Ϊ"1"

	InstructSet(0x25);		//�����Ӵ���2�Ĳ�����Ϊ2400BPS
	printf("uart2_init\n");
	Uart2Coef=0x04;			//���Ӵ���2�ķ�Ƶϵ��Ϊ"4"
	Uart2CoefBuf=0x01;		//��Ƶϵ����ʱ����,��ʼ��Ϊ"1"
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

