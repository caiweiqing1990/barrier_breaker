#ifndef __UART_H__
#define __UART_H__
#define TRUE 1
#define FALSE 0
extern int open_uart_2349(void);
struct serial_t {
    int     fd;
    char    *device;/*/dev/ttyS0,...*/
    int     baud;
    int     databit;/*5,6,7,8*/
    char    parity;/*O,E,N*/
    int     stopbit;/*1,2*/
    int     startbit;/*1*/
    struct termios    options;
};
#endif
