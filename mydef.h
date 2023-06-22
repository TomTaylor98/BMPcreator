#ifndef printf
    #include<stdio.h>
#endif

void print();

int measurement(int **Values);

void BMPcreator(int *Values,int  NumValues);

int FindPid();

void SendViaSocket(int *Values,int NumValues);

void ReceiveViaSocket();
void SignalHandler(int sig);

void SendViaFile(int *values,int NumValues);

void ReceiveViaFile(int sig);