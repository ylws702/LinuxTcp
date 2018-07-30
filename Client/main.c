#include<netinet/in.h>  // sockaddr_in 
#include<unistd.h>
#include<sys/types.h>  // socket 
#include<sys/socket.h>  // socket 
#include<stdio.h>    // printf 
#include<stdlib.h>    // exit 
#include<string.h>    
#include<strings.h>    // bzero 
#include<arpa/inet.h>
#include<pthread.h>

#define SERVER_PORT 5678
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
char send_buffer[BUFFER_SIZE];
char send_msg[FILE_NAME_MAX_SIZE + 1] = "\0";
char send_name[FILE_NAME_MAX_SIZE];
char recv_buffer[BUFFER_SIZE];
char recv_msg[FILE_NAME_MAX_SIZE + 1] = "\0";
char recv_name[FILE_NAME_MAX_SIZE];
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
int client_socket_fd = 0;
socklen_t server_addr_length = sizeof(server_addr);

void send_file(char* file_name, int socket);
void recv_file(char* file_name, int sockfd);
int begain_with(char* str1, char *str2);
void connection();
int senddata();
void* recvdata();

int main()
{

	connection();

	pthread_t thread_id;

	pthread_create(&thread_id, NULL, recvdata, NULL);  //创建线程
	pthread_detach(thread_id); // 线程分离，结束时自动回收资源

	senddata();

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
		bzero(send_buffer, BUFFER_SIZE);
		size_t length = 0;
		// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
		while ((length = fread(send_buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			if (send(socket, send_buffer, length, 0) < 0)
			{
				printf("Send File:%s Failed./n", file_name);
				break;
			}
			bzero(send_buffer, BUFFER_SIZE);
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

	// 从服务器接收数据到send_buffer中 
	// 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止 
	bzero(send_buffer, BUFFER_SIZE);
	ssize_t length = 0;
	while (length = recv(sockfd, send_buffer, BUFFER_SIZE, 0), length > 0)
	{
		if (fwrite(send_buffer, sizeof(char), (size_t)length, fp) < length)
		{
			printf("File:\t%s Write Failed\n", file_name);
			break;
		}
		bzero(send_buffer, BUFFER_SIZE);
	}

	// 接收成功后，关闭文件
	printf("Receive File:\t%s Successful\n", file_name);
	fclose(fp);
}

int begain_with(char *str1, char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return -1;
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);
	if (len1 < len2)
		return -1;
	return memcmp(str1, str2, len2);
}

void connection()
{
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);

	// 创建socket，若成功，返回socket描述符 
	client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

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
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) == 0)
	{
		perror("Server IP Address Error:");
		exit(1);
	}
	server_addr.sin_port = htons(SERVER_PORT);

	// 向服务器发起连接，连接成功后client_socket_fd代表了客户端和服务器的一个socket连接 
	if (connect(client_socket_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		perror("Can Not Connect To Server IP:");
		exit(0);
	}
}

int senddata()
{
	while (1)
	{
		// 输入文件名 并放到缓冲区send_buffer中等待发送 
		bzero(send_msg, FILE_NAME_MAX_SIZE + 1);
		scanf("%s", send_msg);
		bzero(send_buffer, BUFFER_SIZE);
		strncpy(send_buffer, send_msg, strlen(send_msg) > BUFFER_SIZE ? BUFFER_SIZE : strlen(send_msg));

		// 向服务器发送send_buffer中的数据 
		if (send(client_socket_fd, send_buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("File Error:");
			exit(1);
		}

		if (begain_with(send_msg, "file:") == 1)
		{
			strncpy(send_name, send_msg + 5, FILE_NAME_MAX_SIZE - 4);
			send_file(send_name, client_socket_fd);//send file
		}

		if (!strcmp(send_msg, "exit"))
		{
			puts("Connection closed.");
			close(client_socket_fd);
			return 0;
		}
	}

}

void* recvdata()
{
	while (1) {
		// recv函数接收数据到缓冲区recv_buffer中
		bzero(recv_buffer, BUFFER_SIZE);
		if (recv(client_socket_fd, recv_buffer, BUFFER_SIZE, MSG_WAITALL) < 0)
		{
			perror("Server Recieve Data Failed:");
			return((void*)0);
		}
		// 然后从recv_buffer(缓冲区)拷贝到recv_msg中
		bzero(recv_msg, FILE_NAME_MAX_SIZE + 1);
		strncpy(recv_msg, recv_buffer, strlen(recv_buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(recv_buffer));

		printf("server: %s\n", recv_msg);
		if (begain_with(recv_msg, "file:") == 1)
		{
			//send files
			strncpy(recv_name, recv_msg + 5, FILE_NAME_MAX_SIZE - 4);
			recv_file(recv_name, client_socket_fd);
		}
	}
	return((void*)0);
}
