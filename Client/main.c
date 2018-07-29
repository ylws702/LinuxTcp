#include<netinet/in.h>  // sockaddr_in 
#include<unistd.h>
#include<sys/types.h>  // socket 
#include<sys/socket.h>  // socket 
#include<stdio.h>    // printf 
#include<stdlib.h>    // exit 
#include<string.h>    // bzero 
#include<arpa/inet.h>

#define SERVER_PORT 8000 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
char buffer[BUFFER_SIZE];

void send_file(char* file_name, int socket);
void recv_file(char* file_name, int sockfd);
int begain_with(char* str1, char *str2);


int main()
{
	// 声明并初始化一个客户端的socket地址结构 
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);

	// 创建socket，若成功，返回socket描述符 
	int client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket_fd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}

	// 绑定客户端的socket和客户端的socket地址结构 非必需 
	if (-1 == (bind(client_socket_fd, (struct sockaddr*)&client_addr, sizeof(client_addr))))
	{
		perror("Client Bind Failed:");
		exit(1);
	}

	// 声明一个服务器端的socket地址结构，并用服务器那边的IP地址及端口对其进行初始化，用于后面的连接 
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr) == 0)
	{
		perror("Server IP Address Error:");
		exit(1);
	}
	server_addr.sin_port = htons(SERVER_PORT);
	socklen_t server_addr_length = sizeof(server_addr);

	// 向服务器发起连接，连接成功后client_socket_fd代表了客户端和服务器的一个socket连接 
	if (connect(client_socket_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		perror("Can Not Connect To Server IP:");
		exit(0);
	}
	char msg[FILE_NAME_MAX_SIZE + 1]="\0";
	while (strcmp(msg, "exit") != 0)
	{
		// 输入文件名 并放到缓冲区buffer中等待发送 
		bzero(msg, FILE_NAME_MAX_SIZE + 1);
		printf("msg: ");
		scanf("%s", msg);
		bzero(buffer, BUFFER_SIZE);
		strncpy(buffer, msg, strlen(msg)>BUFFER_SIZE ? BUFFER_SIZE : strlen(msg));

		// 向服务器发送buffer中的数据 
		if (send(client_socket_fd, buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("File:");
			exit(1);
		}
		char name[FILE_NAME_MAX_SIZE];
		if (begain_with(msg, "file:") == 1)
		{
			strncpy(name, msg + 5, FILE_NAME_MAX_SIZE - 4);
			send_file(name, client_socket_fd);//send file
		}
	}
	close(client_socket_fd);
	return 0;
}


void send_file(char* file_name, int socket)
{
	// 打开文件并读取文件数据
	FILE *fp = fopen(file_name, "r");
	if (NULL == fp)
	{
		printf("File:%s Not Found\n", file_name);
	}
	else
	{
		bzero(buffer, BUFFER_SIZE);
		int length = 0;
		// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
		while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			if (send(socket, buffer, length, 0) < 0)
			{
				printf("Send File:%s Failed./n", file_name);
				break;
			}
			bzero(buffer, BUFFER_SIZE);
		}

		// 关闭文件
		fclose(fp);
		printf("File:%s Transfer Successful\n", file_name);
	}
}

void recv_file(char* file_name, int sockfd)
{	// 打开文件，准备写入 
	FILE *fp = fopen(file_name, "w");
	if (NULL == fp)
	{
		printf("File:%s Can Not Open To Write\n", file_name);
		exit(1);
	}

	// 从服务器接收数据到buffer中 
	// 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止 
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

	// 接收成功后，关闭文件
	printf("Receive File:\t%s Successful\n", file_name);
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
