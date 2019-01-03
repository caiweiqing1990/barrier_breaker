#ifndef __POWER_MODE_H__
#define __POWER_MODE_H__
#define gprs_on		1
#define gprs_off		2
#define sat_system_reset	3
#define sat_on		4
#define sat_off		5
#define gps_on		6
#define gps_off		7
extern int fd_power;
extern int open_power(void);
#endif
