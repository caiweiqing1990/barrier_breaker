/* sockclnt.c*/  
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h> /*for struct sockaddr_in*/  
#include "socket_client.h"  
static int socket_descriptor;  

static int create_socket(void)
{
        struct sockaddr_in pin;  
        bzero(&pin,sizeof(pin));  
        pin.sin_family = AF_INET;  //一般设置为AF_INET，即使用IP。
        inet_pton(AF_INET,host_name,&pin.sin_addr);  
        pin.sin_port = htons(port); 
        //int socket(int domain//domain：即协议域，, int type, int protocol);
        /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/   
        if((socket_descriptor =  socket(AF_INET,SOCK_STREAM,0)) == -1){  
                perror("error opening socket \n");  
                exit(1);  
        }  
        if(connect(socket_descriptor,(struct sockaddr * )&pin,sizeof(pin)) == -1){  
                perror("error connecting to socket \n");  
                exit(1);  
        }  
        return 0;
}

int link_function(unsigned char CMD[],unsigned char CHECK[],unsigned int send_len,
			    unsigned int revc_len,unsigned int check_len)
{
  
	  int j=0,i;
	 volatile int k=0;
	 int lenSend = 0;
        unsigned char REVC[1024] = {0};   
        printf("sending message to server ..\n");  
#if 1
       for(i=0;i<send_len;i++){
        printf("%02x",CMD[i]);
    } 
     printf("\n");
#endif
        if( (lenSend = send(socket_descriptor,CMD,send_len,0)) == -1 ){  
                perror("error in send \n");  
                exit(1);  
        } else {
			printf("############length of send = %d\n", lenSend);
		}  
  
        printf("..sent message ...wait for message..\n");  
        if( recv(socket_descriptor,REVC,revc_len,0) == -1 ){  
                perror("error in receiving response from server \n");  
                exit(1);  
        }  
	REVC[revc_len] = '\0';
	printf("\nResponse from server:\n");
  	
  	for(j=0;j<revc_len;j++){
       printf("%02x",REVC[j]);
     }  
       printf("\n");
       while((CHECK[k]==REVC[k])&&(k<check_len)){
     		k++;
     		printf("k= %d \t",k);
     	}
        if(k!=check_len){
          printf("\ndata error,Since the %d data segment error\n",k);
          k=0;
          return 0;
          }
        else{
         printf("\nData check success\n");
         k=0;
         return 1;
        }
}

int main(int argc , char * argv[])  
{  
	 volatile int out=1;
        create_socket();
        if(link_function(CONN,CONN_RESP,CONN_LEN,CONN_RESP_LEN,CONN_RESP_CHECK)){
           printf("The current %d message has been successfully sent out\n",(out++));
           if(link_function(HB,HB_RESP,HB_LEN,HB_RESP_LEN,HB_RESP_CHECK)){
         	printf("The current %d message has been successfully sent out\n",(out++));
           }
           else{
              printf("message error,Since the %d message segment error\n",(out++));
           }
        }
        else{
           printf("message error,Since the %d message segment error\n",(out++));
           }      
        close(socket_descriptor);  
        return 1;  
}  

