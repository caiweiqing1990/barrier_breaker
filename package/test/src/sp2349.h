#ifndef __SP2349_H__
#define __SP2349_H__
/*sp_1_uart232_rs485_sat*/
#define RS232_EN	0
#define IRIDIUM_EN	1
#define GPS_EN	2
#define MOTHER_SP1 	4
#define READ_UART0 	5
#define READ_UART1 	6
/*void switch_uart_port(unsigned int cmd);
int serial_interrupt(void);
void InstructSet (unsigned char InstructWord);
void init_serialcomm(void);
*/
extern int open_sp2349(void);
extern int serial_interrupt();
extern unsigned char bUartBusy;
extern char Uart0TBuf[5]; 	//�Ӵ���0���ݷ��ͻ�����(��С�����û���������)
extern char Uart1TBuf[8]; 	//�Ӵ���1���ݷ��ͻ�����(��С�����û���������)
extern char Uart2TBuf[8]; 	//�Ӵ���2���ݷ��ͻ�����(��С�����û���������)
extern unsigned char Uart0TNum,Uart1TNum,Uart2TNum;
#endif
