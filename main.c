#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<dirent.h>
#include<signal.h>
#include<ctype.h>
#include<omp.h>
#include "mydef.h"
#define PORT_NO 3333 

#define BUFSIZE 1024

typedef char* string;

int transfer_mode = 0;
int mode;

int main(int argc,string argv[]){
    signal(SIGUSR1,ReceiveViaFile);
    signal(SIGINT,SignalHandler);
   // signal(SIGUSR1,SignalHandler);
    signal(SIGALRM,SignalHandler);
    int *Values;
    
    if(argc>1){
     if(strcmp(argv[1],"--version")==0){
         omp_set_num_threads(5);
    #pragma omp parallel 
    {
         if(omp_get_thread_num()==1){
             printf("sysprog project version:1.0.0\n");
         }
         if(omp_get_thread_num()==2){
             printf("2022.04.22\n");
         }
         if(omp_get_thread_num()==3){
             printf("made by Tamás Szabó  NK_num(PSK34Q)\n");
         }
          
    }
       
        exit(0);
        
    }
    else if(strcmp(argv[1],"--help")==0){
        printf("commands:\n");
        printf("-file\n");
        printf("-socket\n");
        printf("-send\n");
        printf("receive\n");
        printf("-version\n");
        exit(0);
    }
     else
     {
        for(int i = 1;i<argc;i++)
        {
    
        if(strcmp(argv[i],"-file")==0){
         // printf("to file\n");
          transfer_mode = 0;
         
        }
        if(strcmp(argv[i],"-socket")==0){
         // printf("socket\n");
          transfer_mode = 1;
        }
        if(strcmp(argv[i],"-send")==0){
          mode = 1;
         // printf("send\n");
          
          }
        if(strcmp(argv[i],"-receive")==0){
          mode = 0;
          //printf("receiving.....\n");
          
            }
        }
         if(transfer_mode==0){
           printf("File  ");
           if(mode==0){
             printf("in receive mode\n");
             while(1){

             }
           }
           if(mode==1){
             printf("in send mode\n");
             int val = measurement(&Values);
             SendViaFile(Values,val);
           }
         }
         if(transfer_mode==1){
           printf("Socket  ");
           if(mode==0){
             printf("in receive mode\n");
             ReceiveViaSocket();
           }
           if(mode==1){
             printf("in send mode\n");
             int val = measurement(&Values);
             SendViaSocket(Values,val);
             
           }
      }
    }
  }
else{
    printf("available commands:\n");
        printf("-file\n");
        printf("-socket\n");
        printf("-send\n");
        printf("receive\n");
        printf("-version\n");
        exit(0);
}

  free(Values);
    
   
    
}
