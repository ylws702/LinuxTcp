#include <netinet/in.h> // sockaddr_in
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <stdio.h>    // printf
#include <stdlib.h>   // exit
#include <string.h>   // bzero
#include <pthread.h>

#define SERVER_PORT 5678

#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
char send_buffer[BUFFER_SIZE];
char send_msg[FILE_NAME_MAX_SIZE + 1] = "\0";
char send_name[FILE_NAME_MAX_SIZE];
char recv_buffer[BUFFER_SIZE];
char recv_msg[FILE_NAME_MAX_SIZE + 1] = "\0";
char recv_name[FILE_NAME_MAX_SIZE];
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int server_socket_fd = 0;
int new_server_socket_fd = 0;

void send_file(char* file_name, int socket);
void recv_file(char* file_name, int sockfd);
int begain_with(char *str1, char *str2);
void connection();
int senddata();
void* recvdata();
void deal(int sockfd, pthread_t thread_id);

int main(void)
{

	connection();

	pthread_t thread_id;


	while (1)
	{
		// 定义客户端的socket地址结构
		socklen_t client_addr_length = sizeof(client_addr);

		// 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信
		// accept函数会把连接到的客户端信息写到client_addr中

		puts("Waiting for a connection...");
		new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length);
		puts("Connection established.");


		while (1)
		{
			if (new_server_socket_fd < 0)
			{
				perror("Server Accept Failed:");
				break;
			}

			pid_t childid;
			if (childid = fork() == 0)//子进程
			{
				printf("child process: %d created.\n", getpid());
				close(server_socket_fd);//在子进程中关闭监听  
				deal(new_server_socket_fd, thread_id);//处理监听的连接  
				exit(0);
			}

			break;
		}
	}
	// 关闭监听用的socket
	close(server_socket_fd);
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
		bzero(recv_buffer, BUFFER_SIZE);
		int length = 0;
		// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
		while ((length = fread(recv_buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			if (send(socket, recv_buffer, length, 0) < 0)
			{
				printf("Send File:%s Failed./n", file_name);
				break;
			}
			bzero(recv_buffer, BUFFER_SIZE);
		}

		// 关闭文件
		fclose(fp);
		printf("File:%s Transfer Successful!\n", file_name);
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

	// 从服务器接收数据到recv_buffer中 
	// 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止 
	bzero(recv_buffer, BUFFER_SIZE);
	int length = 0;
	while ((length = recv(sockfd, recv_buffer, BUFFER_SIZE, 0)) > 0)
	{
		if (fwrite(recv_buffer, sizeof(char), length, fp) < length)
		{
			printf("File:\t%s Write Failed\n", file_name);
			break;
		}
		bzero(recv_buffer, BUFFER_SIZE);
	}

	// 接收成功后，关闭文件
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

void connection()
{

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	// 创建socket，若成功，返回socket描述符
	server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket_fd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}
	int opt = 1;
	setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 绑定socket和socket地址结构
	if (-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
	{
		perror("Server Bind Failed:");
		exit(1);
	}

	// socket监听
	if (-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE)))
	{
		perror("Server Listen Failed:");
		exit(1);
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

		// 向客户机发送send_buffer中的数据 
		if (send(new_server_socket_fd, send_buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("File Error:");
			exit(1);
		}

		if (begain_with(send_msg, "file:") == 1)
		{
			strncpy(send_name, send_msg + 5, FILE_NAME_MAX_SIZE - 4);
			send_file(send_name, new_server_socket_fd);//send file
		}

		if (!strcmp(send_msg, "exit"))
		{
			close(new_server_socket_fd);
			puts("Connection closed.");
			return 0;
		}
	}

}

void* recvdata()
{
	pid_t pid = getpid();
	while (1)
	{
		// recv函数接收数据到缓冲区recv_buffer中
		bzero(recv_buffer, BUFFER_SIZE);
		if (recv(new_server_socket_fd, recv_buffer, BUFFER_SIZE, MSG_WAITALL) < 0)
		{
			perror("Server Recieve Data Failed:");
			return((void*)0);
		}
		// 然后从recv_buffer(缓冲区)拷贝到recv_msg中
		bzero(recv_msg, FILE_NAME_MAX_SIZE + 1);
		strncpy(recv_msg, recv_buffer, strlen(recv_buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(recv_buffer));

		printf("client %d: %s\n", pid, recv_msg);

		if (begain_with(recv_msg, "file:") == 1)
		{
			//recv files
			strncpy(recv_name, recv_msg + 5, FILE_NAME_MAX_SIZE - 4);
			recv_file(recv_name, new_server_socket_fd);
		}

		// 关闭与客户端的连接
		if (!strcmp(recv_msg, "exit"))
		{
			return 0;
		}
	}
}

void deal(int sockfd, pthread_t thread_id)
{

	pthread_create(&thread_id, NULL, recvdata, NULL);  //创建线程
	pthread_detach(thread_id); // 线程分离，结束时自动回收资源

	senddata();

	puts("Connection closed.");
}