#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void * print_a(void *a)
{
	for (int i = 0; i < 10; i++) 
	{
		sleep(1);
		printf("a%d\n",i);
	}
	return NULL;
}

// 线程B 方法
void * print_b(void *b) 
{
	for (int i = 0; i<20; i++)
	{
		sleep(1);
		printf("b%d\n", i);
	}
	return NULL;
}

int main()
{
	pthread_t t0,t1;
	pthread_create(&t0, NULL, print_a, NULL);
	pthread_create(&t1, NULL, print_b, NULL);
	getchar();
	//void * result;
	/*pthread_join(t0, &result);
	pthread_join(t1, &result);*/
    return 0;
}