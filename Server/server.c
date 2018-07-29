#include "server.h"

int tcp_init(const char * ip, int port)
{
	/*
	创建通信端点。
	成功完成后，返回一个非负整数套接字文件描述符(socket file descriptor)。
	否则返回-1，并设置errno以指示错误。
	AF_INET:互联网连接
    SOCK_STREAM:TCP连接
	如果protocol参数不为零，则必须指定地址族支持的协议。
	*/
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sfd)
	{
		printf("创建通信端点失败: %s\n", strerror(errno));
		return -1;
	}
	//允许重用本地地址
	int option = 1;
	if (-1 == setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
	{
		printf("允许重用本地地址失败: %s\n", strerror(errno));
		return -1;
	}
	//设置发送缓冲区大小
	option = 256;
	if (-1 == setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &option, sizeof(option)))
	{
		printf("设置发送缓冲区大小失败: %s\n", strerror(errno));
		return -1;
	}
	//设置接收缓冲区大小
	if (-1 == setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &option, sizeof(option)))
	{
		printf("设置接收缓冲区大小失败: %s\n", strerror(errno));
		return -1;
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	//绑定套接字
	if (-1==bind(sfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)))
	{
		printf("绑定套接字失败: %s\n", strerror(errno));
		return -1;
	}
	//侦听套接字连接并限制传入连接的队列
	if (-1 == listen(sfd, MAX_CONN))
	{
		printf("侦听套接字失败: %s\n", strerror(errno));
		return -1;
	}
	puts("侦听套接字成功");
	return 0;
}

int tcp_accept(int sfd)
{
	struct sockaddr_in client_addr;
	int addrlen = sizeof(struct sockaddr);
	int new_fd = accept(sfd, (struct sockaddr*)&client_addr, &addrlen);

	memset(&client_addr, 0, addrlen);

	if (new_fd == -1)
	{
		printf("accept error: %s\n", strerror(errno));
		sleep(1);
		return -1;
	}
	printf("Client%d(%s %d) success connect...\n", 
		new_fd,
		inet_ntoa(client_addr.sin_addr),
		ntohs(client_addr.sin_port));
	return new_fd;
}
