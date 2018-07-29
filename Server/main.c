/*************************************************************************
> File Name: Server.c
> Author: SongLee
************************************************************************/

#include <netinet/in.h> // sockaddr_in
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <stdio.h>    // printf
#include <stdlib.h>   // exit
#include <string.h>   // bzero

#define SERVER_PORT 8000
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
char buffer[BUFFER_SIZE];

void send_file(char* file_name, int socket);
void recv_file(char* file_name, int sockfd);
int begain_with(char *str1, char *str2);

int main(void)
{
	// ��������ʼ��һ���������˵�socket��ַ�ṹ
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	// ����socket�����ɹ�������socket������
	int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket_fd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}
	int opt = 1;
	setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// ��socket��socket��ַ�ṹ
	if (-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
	{
		perror("Server Bind Failed:");
		exit(1);
	}

	// socket����
	if (-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE)))
	{
		perror("Server Listen Failed:");
		exit(1);
	}

	while (1)
	{
		// ����ͻ��˵�socket��ַ�ṹ
		struct sockaddr_in client_addr;
		socklen_t client_addr_length = sizeof(client_addr);

		// �����������󣬷���һ���µ�socket(������)�������socket����ͬ���ӵĿͻ���ͨ��
		// accept����������ӵ��Ŀͻ�����Ϣд��client_addr��
		puts("Waiting for a connection...");
		int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length);
		puts("Connection established.");
		while (1)
		{
			if (new_server_socket_fd < 0)
			{
				perror("Server Accept Failed:");
				break;
			}
			// recv�����������ݵ�������buffer��
			bzero(buffer, BUFFER_SIZE);
			if (recv(new_server_socket_fd, buffer, BUFFER_SIZE, MSG_WAITALL) < 0)
			{
				perror("Server Recieve Data Failed:");
				break;
			}
			// Ȼ���buffer(������)������msg��
			char msg[FILE_NAME_MAX_SIZE + 1];
			bzero(msg, FILE_NAME_MAX_SIZE + 1);
			strncpy(msg, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
			// �ر���ͻ��˵�����
			if (msg[0] == '\0' || strcmp(msg, "exit") == 0)
			{
				close(new_server_socket_fd);
				puts("Connection closed.");
				break;
			}
			printf("client: %s\n", msg);
			char name[FILE_NAME_MAX_SIZE];
			if (begain_with(msg, "send file:") == 1)
			{
				//send files
				strncpy(name, msg + 10, FILE_NAME_MAX_SIZE - 9);
				recv_file(name, new_server_socket_fd);
			}
		}
	}
	// �رռ����õ�socket
	close(server_socket_fd);
	return 0;
}



void send_file(char* file_name, int socket)
{
	// ���ļ�����ȡ�ļ�����
	FILE *fp = fopen(file_name, "r");
	if (NULL == fp)
	{
		printf("File:%s Not Found\n", file_name);
	}
	else
	{
		bzero(buffer, BUFFER_SIZE);
		int length = 0;
		// ÿ��ȡһ�����ݣ��㽫�䷢�͸��ͻ��ˣ�ѭ��ֱ���ļ�����Ϊֹ
		while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			if (send(socket, buffer, length, 0) < 0)
			{
				printf("Send File:%s Failed./n", file_name);
				break;
			}
			bzero(buffer, BUFFER_SIZE);
		}

		// �ر��ļ�
		fclose(fp);
		printf("File:%s Transfer Successful!\n", file_name);
	}
}


void recv_file(char* file_name, int sockfd)
{	// ���ļ���׼��д�� 
	FILE *fp = fopen(file_name, "w");
	if (NULL == fp)
	{
		printf("File:%s Can Not Open To Write\n", file_name);
		exit(1);
	}

	// �ӷ������������ݵ�buffer�� 
	// ÿ����һ�����ݣ��㽫��д���ļ��У�ѭ��ֱ���ļ������겢д��Ϊֹ 
	bzero(buffer, BUFFER_SIZE);
	int length = 0;
	while ((length = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0)
	{
		if (fwrite(buffer, sizeof(char), length, fp) < length)
		{
			printf("File:\t%s Write Failed\n", file_name);
			break;
		}
		bzero(buffer, BUFFER_SIZE);
	}

	// ���ճɹ��󣬹ر��ļ�
	printf("Receive File:\t%s From Server IP Successful\n", file_name);
	fclose(fp);
}

int begain_with(char *str1, char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return -1;
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	if ((len1 < len2) || (len1 == 0 || len2 == 0))
		return -1;
	char *p = str2;
	int i = 0;
	while (*p != '\0')
	{
		if (*p != str1[i])
			return 0;
		p++;
		i++;
	}
	return 1;
}
