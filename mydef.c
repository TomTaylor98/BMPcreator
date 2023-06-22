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



int measurement(int **Values){

    srand(time(NULL));
    float P1 = 42.8571f;
    float P2 = ((float)11/31)*100;
    int x  = 0;
    time_t tiime = time(NULL);
    int seconds = tiime%(60*15);

    *Values = malloc(seconds*sizeof(int));

    for(int i = 0;i<seconds;i++){
        double num = (100-0)*(double)rand()/RAND_MAX;   
       if (num< P1){
           x = x+1;
       }
       else if(num<P1+P2&& P1<num){
           x = x-1;
       }
       else {
           x = x;
       }
       Values[0][i] = x;
    }
    
     return seconds;
}
  
void BMPcreator(int *Values,int  NumValues){
    
    /*
   printf("Number of Values: %d\n",NumValues);
   printf("Values:\n");
   for(int i = 0;i<NumValues;i++){
       printf("%d ",Values[i]);
   }
   */
    int bp; 
    bp = open("bi.bmp",O_CREAT|O_TRUNC|O_WRONLY);
    int height = NumValues;
    int width = NumValues;
    int rowsize;
    unsigned int ONE = 0b10000000000000000000000000000000;
    int middle;
    if(width%32>0){
        rowsize = width/32+1;
          }
    else{
        rowsize = width/32;
          }
    
    if((height%2)==0){
        middle = height/2;
    }
    else{
        middle = height/2+1;
    }
    int headerSize = 62;
    int pixelarraysize =height*rowsize*sizeof(unsigned int);
    int FileSize = headerSize + pixelarraysize;

    unsigned char header[62] = {0x42, 0x4d, FileSize, FileSize>>8, FileSize>>16, FileSize>>24, 0x00, 0x00, 0x00, 0x00,
    0x3e, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, width, width>>8, width>>16, width>>24, height, height>>8,
    height>>16, height>>24, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x61, 0x0f, 0x00, 0x00, 0x61, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0x00, 0x0, 0xff, 0x00};

    unsigned int *pixelarray = calloc(pixelarraysize,sizeof(unsigned int));
   
    int j = 0;
    for(int i =0;i<width;i++){
         pixelarray[middle*rowsize +(Values[i]-1)*rowsize + j] = pixelarray[middle*rowsize + (Values[i]-1)*rowsize +j]|ONE>>(i%32);
         if((i+1)%32==0){
             j++;
         }
    }
  
    for(int i = 0;i<height*rowsize;i++){
        int big_end[4];
        big_end[0] = (pixelarray[i]&0xff000000)>>24;
        big_end[1] = (pixelarray[i]&0x00ff0000)>>8;
        big_end[2] = (pixelarray[i]&0x0000ff00)<<8;
        big_end[3] = (pixelarray[i]&0x000000ff)<<24;
        pixelarray[i] = big_end[0]|big_end[1]|big_end[2]|big_end[3];
    }

    write(bp,header,62*sizeof(char));
    write(bp,pixelarray,pixelarraysize*sizeof(u_int32_t));
    free(pixelarray);
    //free(Values);
    close(bp);
    
}
int FindPid(){
  
  int PID_num = 0;
  DIR *d;
  DIR *d2;
  bool found_pid = false;
  struct dirent *entry;
  struct dirent *entry_2;
  chdir("/proc");
  d = opendir(".");
  while((entry=readdir(d))!=NULL){
    if(isdigit((*entry).d_name[0])!=0){
      chdir("/proc");
      chdir((*entry).d_name);

      d2 = opendir(".");

      while((entry_2=readdir(d2))!=NULL){
        if(strcmp((*entry_2).d_name,"status")==0)
        {
        FILE *f = fopen((*entry_2).d_name,"r");
        char str[BUFSIZE];
        fscanf(f,"Name:\t%s\n",str);

        if(strcmp(str,"chart")==0){
            //printf("megvan in file:%s\n",(*entry).d_name);
            char line[BUFSIZE];
            char* token;
            while(fgets(line,100,f)!=NULL){
                token = strtok(line,"\t");
                if(strcmp(token,"Pid:")==0){
                  token = strtok(NULL,"\t");
                 
                  if(atoi(token)!=getpid()){
                     PID_num = atoi(token);
                  found_pid = true;
                  }
                }  
            }
        }
        fclose(f);
        }     
          }
    }
  }
  
  if(found_pid){
    return PID_num;
  }
  else{
    return -1;
  }
 
}  

void SendViaSocket(int *Values,int NumValues){
    
    int s;                          
   int bytes;
   int bytes_rec;                        // received/sent bytes
   int flag;                         // transmission flag
   char on;                          // sockopt option

   int buffer[1] = {NumValues};
   int buffer_rec[1];
    
   int size_of_Values = NumValues*sizeof(int);
   //printf("%d\n",size_of_Values);
   unsigned int server_size;        
   struct sockaddr_in server;       
                      
    int num_values;                    

  
   on   = 1;
   flag = 0;
   server.sin_family      = AF_INET;
   server.sin_addr.s_addr = inet_addr("127.0.0.1");
   server.sin_port        = htons(PORT_NO);
   server_size = sizeof server;

   /************************ Creating socket *******************/
   s = socket(AF_INET, SOCK_DGRAM, 0 );
   if ( s < 0 ) {
     // fprintf(stderr, " %s: Socket creation error.\n", argv[0]);
      exit(2);
      }
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
   setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

   bytes = sendto(s, buffer, sizeof(buffer) , flag, (struct sockaddr *) &server, server_size);
   
   printf (" %i bytes have been sent to server.\n", bytes);
    
   bytes_rec = recvfrom(s, buffer_rec, sizeof(buffer_rec), flag, (struct sockaddr *) &server, &server_size);
   
   if(buffer[0]==buffer_rec[0]&&bytes==bytes_rec){
      printf("OK\n");
   }
   else{
      printf("HIBA! a visszakapott értékek nem egyeznek a küldöttel!");
      exit(3);
      }
   sleep(2);

   bytes = sendto(s, Values, size_of_Values, flag, (struct sockaddr *) &server, server_size);
   printf("I have sent %d bytes\n",bytes);
   bytes_rec = recvfrom(s, buffer, sizeof(buffer), flag, (struct sockaddr *) &server, &server_size);
   if(buffer[0]==size_of_Values){
      printf("OK\n");
   }
   else{
      printf("HIBA! a kapott byte mennyiség nem egyezik a küldöttel");
      exit(4);
   }
   close(s);
}

void ReceiveViaSocket(){
    int bytes;      
   int bytes_rec;                  // received/sent bytes
   int err;                          // error code
   int flag;                         // transmission flag
   char on;                          // sockopt option
   int buffer[1];             // datagram buffer area
   unsigned int server_size;         // length of the sockaddr_in server
   unsigned int client_size;         // length of the sockaddr_in client
   struct sockaddr_in server;        // address of server
   struct sockaddr_in client;        // address of client
   char input[BUFSIZE];
   int *Values;
    int s;
   /************************ Initialization ********************/
   on   = 1;
   flag = 0;
   server.sin_family      = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port        = htons(PORT_NO);
   server_size = sizeof server;
   client_size = sizeof client;
  
   int NumValues;

   /************************ Creating socket *******************/
   s = socket(AF_INET, SOCK_DGRAM, 0 );
   if ( s < 0 ) {
      exit(2);
      }
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
   setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

   /************************ Binding socket ********************/
   err = bind(s, (struct sockaddr *) &server, server_size);
   if ( err < 0 ) {
    
      exit(3);
      }

   while(1){ // Continuous server operation
      /************************ Receive data **********************/
      printf("\n Waiting for data...\n");
      bytes = recvfrom(s, buffer, sizeof(buffer), flag, (struct sockaddr *) &client, &client_size );
      
      printf("I have received %d bytes\n",bytes);
     
      
      NumValues = buffer[0];
      Values = malloc(sizeof(int)*buffer[0]);
      /************************ Sending data **********************/
     
      bytes = sendto(s, buffer, sizeof(buffer), flag, (struct sockaddr *) &client, client_size);
      
      printf("waiting for another message\n");
      bytes_rec = recvfrom(s, Values, sizeof(int)*buffer[0], flag, (struct sockaddr *) &client, &client_size );
      printf("I have received %d bytes\n",bytes_rec);
      
      buffer[0] = bytes_rec;
      bytes = sendto(s, buffer, sizeof(buffer), flag, (struct sockaddr *) &client, client_size);
      
      BMPcreator(Values,NumValues);
        
      free(Values);
      
      }
}
      
    void SignalHandler(int sig){
        if(sig==SIGINT){
            printf("Goodbye\n");
            exit(0);
        }
        if(sig==SIGUSR1){
            printf("data transfer through file is not available\n");
        }
        if(sig==SIGALRM){
            printf("server currently not responding\n");
        }
    }

    void SendViaFile(int *values,int NumValues){
        printf("mode:send Via:file\n");
        int pid_num = 0;
        char filename[BUFSIZE];
        sprintf(filename,"/home/%s/Measurements.txt",getlogin());
 
        FILE *f = fopen(filename,"w");

        for(int i = 0;i<NumValues;i++){
        fprintf(f,"%d\n",values[i]);
        }
        pid_num = FindPid();
       // printf("%d\n",pid_num);
        if(pid_num!=-1){
    
            kill(pid_num,SIGUSR1);
        }
    fclose(f);
}

void ReceiveViaFile(int sig){
  if(sig==SIGUSR1){
   
    char filename[BUFSIZE];
    char buffer[BUFSIZE];
  sprintf(filename,"/home/%s/Measurements.txt",getlogin());
  FILE *f = fopen(filename,"r");
  int j = 0;
  int *Values = malloc(sizeof(int));
  while(fgets(buffer,BUFSIZE,f)!=NULL){
    Values[j] = atoi(buffer);
    j++;
    Values = realloc(Values,(j+1)*sizeof(int)); 
  }
  BMPcreator(Values,j);
 free(Values);
  }
  exit(1);
}