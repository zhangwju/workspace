/***************************************
 * Filename: test.c 
 * Author: zhangwj
 * Description: a simple queue
 * Date: 2017-05-16
 *
 **************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
	int *ptr;
	int front;
	int rear;
	int maxsize;
}QNode, *QPtr;

bool QueueFull(QPtr Q)
{
	if (Q->front == ((Q->rear + 1)% Q->maxsize)) {
		return false;
	}
	
	return true;	
}

bool QueueEmpty(QPtr Q)
{
	if (Q->front == Q->rear) {
		return true;
	}

	return false;
}

bool DeQueue(QPtr Q, int *val)
{
	if (QueueEmpty(Q)) {
		return false;
	}

	*val = Q->ptr[Q->front];
	Q->front = (Q->front + 1) % Q->maxsize;
	return 0;
}

bool EQueue(QPtr Q, int val)
{
	if (!QueueFull(Q)) {
		return false;
	}
	Q->ptr[Q->rear] = val;
	Q->rear = (Q->rear + 1) % Q->maxsize;
	
	return true; 
}

void InitQueue(QPtr Q, int maxsize)
{
	Q->ptr = (int *)malloc(sizeof(int) *maxsize);
	if(NULL == Q->ptr) {
		exit(-1);
	}
	
	Q->front = 0;
	Q->rear = 0;
	Q->maxsize = maxsize;	
}

void PrintQueue(QPtr Q)
{
	int i = Q->front;

	printf("Q->front: %d\n", Q->front);
	printf("Q->rear: %d\n", Q->rear);
	printf("queue:\n");
	while ((i % Q->maxsize) != Q->rear) {
		printf("%d ", Q->ptr[i]);
		i++;
	}
	
	printf("\n");
}

int main()
{
	int val;
	QNode Q;
	InitQueue(&Q, 20);
	EQueue(&Q, 1);
	EQueue(&Q, 5);
	EQueue(&Q, 6);
	EQueue(&Q, 8);
	EQueue(&Q, 20);
	PrintQueue(&Q);

	DeQueue(&Q, &val);
	printf("del: %d\n", val);
	PrintQueue(&Q);


	return 0;
}
