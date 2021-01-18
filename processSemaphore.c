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

//�������Ķ���
typedef struct{
	int length;
	int data[BUFF_SIZE];
	int oddNum;
	int evenNum;
}Buff;

// �����ź����ṹ
union semaphore{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

/*��������*/
int init(int id,int val){
	union semaphore sem;
	sem.val = val; 
	if(semctl(id,0,SETVAL,sem)==-1){
		printf("ʧ�� \n");
		return -1;
	}
	return 0;
}

/*ɾ������/
int del(int id){
	union semaphore sem;
	if(semctl(id,0,IPC_RMID,sem)==-1){
		return -1;
	}
	return 0;
}

/*p����*/
int P(int id){ //�ź���-1
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(id,&sem_b,1)==-1){
		return -1;
	}
	return 0;
}

/*v����*/
int V(int id){ //�ź���+1
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(id,&sem_b,1)==-1){
		return -1;
	}
	return 0;
}

//���ż��
void geteven(Buff *buff){
	// buff ����Ϊ1
	int i;
	for(i = 0; i < buff->length; i++){
		if(0 == (buff->data[i]%2)){
			printf("��õ�ż��:%i \n",buff->data[i]);
			// ��������ֵɾ�����������������λ
			int j;
			for(j = i; j < buff->length -1; j++){
				buff->data[j] = buff->data[j+1];
			}
			buff->length--;
			break;
		}
	}
}

//ͳ��ż��
void counteven(Buff *buff){
	int i;
	for(i = 0; i < buff->length; i++){
		if(0 == (buff->data[i]%2)){
		buff->evenNum++;}
	printf("ż������Ŀ:%i \n",buff->evenNum);
}

//����һ����
void produce(Buff *buff){
	int temp = random(10);
	buff->data[buff->length] = temp;
	buff->length++;
	printf("����һ����:%i \n",temp);
	sleep(1);
}

//�������
void getodd(Buff *buff){
	int i;
	for(i = 0; i < buff->length; i++){
		if(1 == (buff->data[i]%2)){

			printf("��õ�����:%i \n",buff->data[i]);
			int j;
			for(j = i; j < buff->length -1; j++){
				buff->data[j] = buff->data[j+1];
			}
			buff->length--;
			break;
		}
	}
}

//ͳ������
void countodd(Buff *buff){
	int i;
	for(i = 0; i < buff->length; i++){
		if(1 == (buff->data[i]%2)){
		buff->oddNum++;}
	printf("��������Ŀ:%i \n",buff->oddNum);
}

int main(int arg,char* argv[]){
	Buff *buff;
	pid_t pid;
	int shmid,empty,full;
	shmid = shmget(MY_SHM_ID,sizeof(Buff),0666|IPC_CREAT);
	empty = semget(MY_SEM_KEY1,1,0666|IPC_CREAT);
	full = semget(MY_SEM_KEY2,1,0666|IPC_CREAT);
	if(shmid == -1 || empty == -1 || full == -1){
		printf("����");
	}
	init(empty,10);
	init(full,0);

	//��ʼ��������
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
		printf("����ʧ��");
	}else if(0 == pid){
		if(i==0){
			printf("�ӽ���P1�������С�����\n");
			int j;
			for(j = 0; j < 8; j++){
				P(empty);
				produce(buff);
				V(full);
			}
		}
		if(i==1){
			printf("�ӽ���P2�������С�����\n");
			int j;
			for(j = 0; j < 2; j++){
				P(full);
				geteven(buff);
				counteven(buff);
				V(empty);
			}
		}
		if(i==2){
			printf("�ӽ���P3�������С�����\n");
			int j;
			for(j = 0; j < 2; j++){
				P(full);
				getodd(buff);
				countodd(buff);
				V(empty);
			}
		}
	}else{
		printf("�������!");
		sleep(10);
		del(empty);
		del(full);
	}
	return 0;
} 
