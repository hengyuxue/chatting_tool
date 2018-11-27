#ifndef _SOCKET_H
#define _SOCKET_H
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include<wait.h>
#include<mysql/mysql.h>
#define NORMAL 0
#define BANNED 1
#define GROUP 2
#define PRIVATE 3
#define LORD 4
#define CROWD 5
#define NOTLOGIN 6
#define ISEXIST 7
#define NOTEXIST 8
#define ISTRUE 9
#define ISFALSE 10
#define KICK 11
#define SEARCHNUM 12
#define PROM 13
#define MSG_FILENAME 14
#define MSG_CONTENT 15
#define MSG_DONE 16
#define FILE_TRANS 17
#define FILE_SENDS 18
typedef struct inf{
	int flag;//flag=0意为注册，flag=1意为登录
	int p_exist;//ISEXIST or NOTEXIST
	char usr_name[40];
	char usr_passwd[40];	
	char question[100];
	char answer[100];
	char str[100];
//	int correct;//ISTRUE or ISFALSE
}Account_inf;
typedef struct Online_usrs{
	char usr_name[40];
	int connfd;
	int flag;//NORMAL or BANNED
	int idenity;// LORD or CROWD or NOTLOGIN
}Onlines; 
typedef struct MESSAGE{
	int flag;//PRIVATE or GROPR
	int op;
	int state;//KICK
	int func;//SEARCHNUM
	int prompt;
	char str[200];
	char recv_usr[40];
	int type;//file_trans
	char ver_recv[5];
}MSG;
typedef struct Trans_file
{
	int type;
	char content[1024];
}File_inf;
#endif
