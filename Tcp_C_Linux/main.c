#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>
#include <errno.h>


#define MAX_CON 10
#define MAX_SIZE 1024
#define CMD_EXIT "exit"
#define CMD_DOWNLOAD_FILE "df"

int main()
{
    printf("hello from Tcp_C_Linux!\n");
    return 0;
}