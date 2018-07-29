#include "server.h"

int tcp_init(const char * ip, int port)
{
	/*
	����ͨ�Ŷ˵㡣
	�ɹ���ɺ󣬷���һ���Ǹ������׽����ļ�������(socket file descriptor)��
	���򷵻�-1��������errno��ָʾ����
	AF_INET:����������
    SOCK_STREAM:TCP����
	���protocol������Ϊ�㣬�����ָ����ַ��֧�ֵ�Э�顣
	*/
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sfd)
	{
		printf("����ͨ�Ŷ˵�ʧ��: %s\n", strerror(errno));
		return -1;
	}
	//�������ñ��ص�ַ
	int option = 1;
	if (-1 == setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
	{
		printf("�������ñ��ص�ַʧ��: %s\n", strerror(errno));
		return -1;
	}
	//���÷��ͻ�������С
	option = 256;
	if (-1 == setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &option, sizeof(option)))
	{
		printf("���÷��ͻ�������Сʧ��: %s\n", strerror(errno));
		return -1;
	}
	//���ý��ջ�������С
	if (-1 == setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &option, sizeof(option)))
	{
		printf("���ý��ջ�������Сʧ��: %s\n", strerror(errno));
		return -1;
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	//���׽���
	if (-1==bind(sfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)))
	{
		printf("���׽���ʧ��: %s\n", strerror(errno));
		return -1;
	}
	//�����׽������Ӳ����ƴ������ӵĶ���
	if (-1 == listen(sfd, MAX_CONN))
	{
		printf("�����׽���ʧ��: %s\n", strerror(errno));
		return -1;
	}
	puts("�����׽��ֳɹ�");
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
