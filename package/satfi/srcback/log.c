#include<stdio.h>
#include<time.h>
#include<sys/timeb.h>
#include<syslog.h>
#include<stdarg.h>
#include"log.h"

#define HT_ERR_LOG_TAG "hterr"
#define HT_ERR_LOG_PRI LOG_INFO 

#define HT_NOR_LOG_TAG "htlog"
#define HT_NOR_LOG_OPTION LOG_PID|LOG_NDELAY
#define HT_NOR_LOG_FAC LOG_LOCAL0
#define HT_NOR_LOG_PRI LOG_INFO 
#define HT_LOG_TYPE_TAG_LEN 64

void ErrorLog(char *pLog, const char *pFunc, int dwLine)
{
  char ucSysLog[512];

  sprintf(ucSysLog,"Func:%s,Line:%d,Desc:%s",pFunc,dwLine,pLog);

  openlog(HT_ERR_LOG_TAG,HT_NOR_LOG_OPTION,HT_NOR_LOG_FAC);
  syslog(HT_ERR_LOG_PRI,ucSysLog);
  closelog();

  return;
}

void NormalLog(char *pLog)
{
  openlog(HT_NOR_LOG_TAG,HT_NOR_LOG_OPTION,HT_NOR_LOG_FAC);
  syslog(HT_NOR_LOG_PRI,pLog);
  closelog();
  return;
}

void Sq_Rcv_Log( int dwQid, unsigned char *pBuf, int Len)
{
  return;
}

void Sq_Send_Log( int dwQid, unsigned char *pBuf, int Len)
{
  return;
}

#define __HT_DEBUG__

void satfi_log_printf(const char *fmt,...)
{
#ifdef __HT_DEBUG__
  va_list args;
  va_start(args, fmt);
  char ucBuf[1024];
  vsnprintf(ucBuf, 1024, fmt, args);
  NormalLog(ucBuf);
  va_end(args);
#endif
}


