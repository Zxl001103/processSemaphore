#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MY_SHM_ID 9956
#define MY_SEM_KEY1 9957
#define MY_SEM_KEY2 9958
#define BUFF_SIZE 255
#define random(x) (rand()%x)
#ifndef MYSEM_H_
#define MYSEM_H_

//缓冲区的定义
typedef struct{
	int length;
	int data[BUFF_SIZE];
	int oddNum;
	int evenNum;
}Buff;

// 定义信号量结构
union semaphore{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

/*声明函数*/
int init(int id,int val){
	union semaphore sem;
	sem.val = val; 
	if(semctl(id,0,SETVAL,sem)==-1){
		printf("失败 \n");
		return -1;
	}
	return 0;
}

/*删除操作/
int del(int id){
	union semaphore sem;
	if(semctl(id,0,IPC_RMID,sem)==-1){
		return -1;
	}
	return 0;
}

/*p操作*/
int P(int id){ //信号量-1
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(id,&sem_b,1)==-1){
		return -1;
	}
	return 0;
}

/*v操作*/
int V(int id){ //信号量+1
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(id,&sem_b,1)==-1){
		return -1;
	}
	return 0;
}

//获得偶数
void geteven(Buff *buff){
	// buff 长度为1
	int i;
	for(i = 0; i < buff->length; i++){
		if(0 == (buff->data[i]%2)){
			printf("获得的偶数:%i \n",buff->data[i]);
			// 将索引的值删除，所有索引后的退位
			int j;
			for(j = i; j < buff->length -1; j++){
				buff->data[j] = buff->data[j+1];
			}
			buff->length--;
			break;
		}
	}
}

//统计偶数
void counteven(Buff *buff){
	int i;
	for(i = 0; i < buff->length; i++){
		if(0 == (buff->data[i]%2)){
		buff->evenNum++;}
	printf("偶数的数目:%i \n",buff->evenNum);
}

//生产一个数
void produce(Buff *buff){
	int temp = random(10);
	buff->data[buff->length] = temp;
	buff->length++;
	printf("生产一个数:%i \n",temp);
	sleep(1);
}

//获得奇数
void getodd(Buff *buff){
	int i;
	for(i = 0; i < buff->length; i++){
		if(1 == (buff->data[i]%2)){

			printf("获得的奇数:%i \n",buff->data[i]);
			int j;
			for(j = i; j < buff->length -1; j++){
				buff->data[j] = buff->data[j+1];
			}
			buff->length--;
			break;
		}
	}
}

//统计奇数
void countodd(Buff *buff){
	int i;
	for(i = 0; i < buff->length; i++){
		if(1 == (buff->data[i]%2)){
		buff->oddNum++;}
	printf("奇数的数目:%i \n",buff->oddNum);
}

int main(int arg,char* argv[]){
	Buff *buff;
	pid_t pid;
	int shmid,empty,full;
	shmid = shmget(MY_SHM_ID,sizeof(Buff),0666|IPC_CREAT);
	empty = semget(MY_SEM_KEY1,1,0666|IPC_CREAT);
	full = semget(MY_SEM_KEY2,1,0666|IPC_CREAT);
	if(shmid == -1 || empty == -1 || full == -1){
		printf("错误");
	}
	init(empty,10);
	init(full,0);

	//初始化缓冲区
	buff = (Buff*)shmat(shmid,NULL,0);
	buff->length = 0;
	buff->oddNum = 0;
	buff->evenNum = 0;

	int i;
	for(i = 0; i < 3; i++){
		pid = fork();
		if(pid == 0 || pid < 0) break;
	}
	if(pid < 0){
		printf("创建失败");
	}else if(0 == pid){
		if(i==0){
			printf("子进程P1正在运行。。。\n");
			int j;
			for(j = 0; j < 8; j++){
				P(empty);
				produce(buff);
				V(full);
			}
		}
		if(i==1){
			printf("子进程P2正在运行。。。\n");
			int j;
			for(j = 0; j < 2; j++){
				P(full);
				geteven(buff);
				counteven(buff);
				V(empty);
			}
		}
		if(i==2){
			printf("子进程P3正在运行。。。\n");
			int j;
			for(j = 0; j < 2; j++){
				P(full);
				getodd(buff);
				countodd(buff);
				V(empty);
			}
		}
	}else{
		printf("程序结束!");
		sleep(10);
		del(empty);
		del(full);
	}
	return 0;
} 
