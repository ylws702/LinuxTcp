#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

//最大侦听队列长度
#define MAX_CONN SOMAXCONN

#define MAX_SIZE 1024

#define CMD_EXIT "exit"

#define CMD_DOWNLOAD_FILE "df"

typedef enum  tagCmdID
{
	CMD_INVALID = -1,
	CMD_FILE_EXIST,
	CMD_FILE_NOT_EXIST
}E_CMD_ID;

typedef struct tagClientCom
{
	E_CMD_ID cmd_id;
	long length;
}T_CLIENT_COM_HEADER;

typedef struct
{
	int sfd;
	int cfd;
	char filename[128];
}tcp_info;

int tcp_init(const char* ip, int port);
int tcp_accept(int sfd);
void * send_file(void * arg);
void * pthread_recv(void *arg);
void signalhandler(void);
