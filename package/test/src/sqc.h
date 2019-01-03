#ifndef __SQC_H__
#define __SQC_H__
#define GPS 1 
#define GPRS 0
#define CSQ_tenths 6
#define CSQ_demarcation '5'
#define cmd_3732_string "ifup wan"  
#define cmd_2500_string "ifup wan6" 
#define BAUDRATE B115200  
#define BAUDRATE_GPS B38400 
#define MODEMDEVICE "/dev/ttyS0"  
#define R_BUF_LEN (256)  
void printtid(void);
int uart_pthread(void);
#endif
