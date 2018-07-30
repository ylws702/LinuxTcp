#include <netinet/in.h> // sockaddr_in
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <stdio.h>    // printf
#include <stdlib.h>   // exit
#include <string.h>
#include <strings.h>   // bzero
#include <pthread.h>


#define SERVER_PORT 5679

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
int server_socket_fd = 0;

void send_file(char* file_name, int socket);
void recv_file(char* file_name, int sockfd);
int begin_with(char *str1, char *str2);
void connection();
int senddata(int socket_fd);
void* recv_thread(void* args);
void* listen_thread();

typedef struct client_info_node
{
	struct sockaddr_in client_addr;
	int socket_fd;
	pthread_t conn_thread_id;
	struct client_info_node* next;
}node;

node* clients = NULL;
int clients_using = 0;
//连接客户端数量
size_t client_count = 0;
#define MAX_CLIENT 20

int list_add(const node* client);
int list_remove(int socket_fd);

int main(void)
{

	connection();
	pthread_t listen_thread_id;

	pthread_create(&listen_thread_id, NULL, listen_thread, NULL);  //创建监听线程
	pthread_detach(listen_thread_id); // 线程分离，结束时自动回收资源
	node* cur;
	while (1)
	{
		// 输入文件名 并放到缓冲区send_buffer中等待发送 
		bzero(send_msg, FILE_NAME_MAX_SIZE + 1);
		scanf("%s", send_msg);
		bzero(send_buffer, BUFFER_SIZE);
		strncpy(send_buffer, send_msg, strlen(send_msg) > BUFFER_SIZE ? BUFFER_SIZE : strlen(send_msg));
		while (clients_using)
		{
			sleep(1);
		}
		clients_using = 1;
		cur = clients;
		while (cur!=NULL)
		{
			senddata(cur->socket_fd);
			cur = cur->next;
		}
		clients_using = 0;
	}
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
		size_t length = 0;
		// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
		while (length = fread(recv_buffer, sizeof(char), BUFFER_SIZE, fp),
			length > 0)
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
	ssize_t length = 0;
	while (length = recv(sockfd, recv_buffer, BUFFER_SIZE, 0)
		, length > 0)
	{
		if (fwrite(recv_buffer, sizeof(char), (size_t)length, fp) < length)
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

int begin_with(char *str1, char *str2)
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

int senddata(int socket_fd)
{
	// 向客户机发送send_buffer中的数据 
	if (send(socket_fd, send_buffer, BUFFER_SIZE, 0) < 0)
	{
		perror("File Error:");
		exit(1);
	}

	if (begin_with(send_msg, "file:") == 1)
	{
		strncpy(send_name, send_msg + 5, FILE_NAME_MAX_SIZE - 4);
		send_file(send_name, socket_fd);//send file
	}

	if (!strcmp(send_msg, "exit"))
	{
		close(socket_fd);
		puts("Connection closed.");
		return 0;
	}
}

void* recv_thread(void* sfd)
{
	int socket_fd = *(int*)sfd;
	while (1)
	{
		// recv函数接收数据到缓冲区recv_buffer中
		bzero(recv_buffer, BUFFER_SIZE);
		if (recv(socket_fd, recv_buffer, BUFFER_SIZE, MSG_WAITALL) < 0)
		{
			perror("Server Recieve Data Failed:");
			return((void*)0);
		}
		// 然后从recv_buffer(缓冲区)拷贝到recv_msg中
		bzero(recv_msg, FILE_NAME_MAX_SIZE + 1);
		strncpy(recv_msg, recv_buffer, strlen(recv_buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(recv_buffer));

		printf("client %d: %s\n", socket_fd, recv_msg);

		if (begin_with(recv_msg, "file:") == 1)
		{
			//recv files
			strncpy(recv_name, recv_msg + 5, FILE_NAME_MAX_SIZE - 4);
			recv_file(recv_name, socket_fd);
		}

		// 关闭与客户端的连接
		if (strcmp(recv_msg, "exit") == 0)
		{
			list_remove(socket_fd);
			shutdown(socket_fd, SHUT_RDWR);
			printf("Connection with client %d closed, %d clients remained.\n", socket_fd, client_count);
			return NULL;
		}
	}
}

void* listen_thread()
{
	node client;
	socklen_t client_addr_length;
	while (1)
	{
		//超过连接数量,等待
		while (client_count >= MAX_CLIENT)
		{
			sleep(1);
		}
		//client_addr长度
		client_addr_length = sizeof(client.client_addr);
		// 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信
		// accept函数会把连接到的客户端信息写到client_addr中
		puts("Waiting for a new connection...");
		client.socket_fd = accept(server_socket_fd, (struct sockaddr*)&client.client_addr, &client_addr_length);
		printf("A new connection accepted, socket file descriptor: %d\n", client.socket_fd);
		if (client.socket_fd < 0)
		{
			perror("Server Accept Failed:");
			continue;
		}
		list_add(&client);
		//创建线程
		if (pthread_create(&client.conn_thread_id, NULL, recv_thread, &client.socket_fd)!=0)
		{
			puts("New thread created failed!");
		}
		printf("Connection established, socket file descriptor: %d\n", client.socket_fd);
		printf("Now %d clients connecting.\n", client_count);
		printf("New thread created, thread_id: %lX\n", client.conn_thread_id);
		pthread_detach(client.conn_thread_id); // 线程分离，结束时自动回收资源
	}
	//无法访问的代码
	//// 关闭监听用的socket
	//close(server_socket_fd);
}

int list_add(const node* client)
{
	while (clients_using)
	{
		sleep(1);
	}
	if (client_count >= MAX_CLIENT)
	{
		return -1;
	}
	clients_using = 1;
	if (0==client_count)
	{
		clients = (node*)malloc(sizeof(node));
		clients->client_addr = client->client_addr;
		clients->socket_fd = client->socket_fd;
		clients->conn_thread_id = client->conn_thread_id;
		clients->next = NULL;
		client_count++;
		clients_using = 0;
		return 0;
	}
	node* p = clients;
	node* cur = p->next;
	while (cur != NULL)
	{
		if (client->socket_fd == p->socket_fd)
		{
			clients_using = 0;
			return -1;
		}
		p = cur;
		cur = cur->next;
	}
	p->next = (node*)malloc(sizeof(node));
	cur = p->next;
	cur->client_addr = client->client_addr;
	cur->socket_fd = client->socket_fd;
	cur->conn_thread_id = client->conn_thread_id;
	cur->next = NULL;
	client_count++;
	clients_using = 0;
	return 0;
}

int list_remove(int socket_fd)
{
	while (clients_using)
	{
		sleep(1);
	}
	if (0 == client_count)
	{
		return -1;
	}
	clients_using = 1;
	node* p = clients;
	if (1 == client_count)
	{
		if (p->socket_fd != socket_fd)
		{
			printf("Remove client %d error: not found!", socket_fd);
			clients_using = 0;
			return -1;
		}
		else
		{
			free(p);
			p = NULL;
			client_count--;
			clients_using = 0;
			return 0;
		}
	}
	node* cur = p->next;
	while (cur != NULL)
	{
		if (socket_fd == cur->socket_fd)
		{
			p->next = cur->next;
			free(cur);
			client_count--;
			clients_using = 0;
			return -1;
		}
		p = cur;
		cur = cur->next;
	}
	printf("Remove client %d error: not found!", socket_fd);
	clients_using = 0;
	return -1;
}
