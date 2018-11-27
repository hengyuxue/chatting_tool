#include"socket.h"
#define MYPORT 7100
#define MAXLINK 100
#define MAXUSR 50
#define HOST "localhost"
#define USERNAME "root"
#define PASSWD "123456"
#define DATABASE "chatting_inf"
MYSQL mysql;//一个数据库结构体
MYSQL_RES *res;//一个结果集结构体
MYSQL_ROW row; //char** 二维数组，存放一条条记录
//int sockfd;
int connfds[MAXLINK];
Onlines usrs[MAXUSR];
//Account_inf usr;
char query[100];
int init_socket()
{
	int sockfd;
	int sock_bind;
	struct sockaddr_in saddr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		perror("create socket fail");
		exit(-1);
	}
	bzero(&saddr,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(MYPORT);
	saddr.sin_addr.s_addr = inet_addr("192.168.100.49");
	sock_bind = bind(sockfd,(struct sockaddr*)&saddr,sizeof(struct sockaddr));
	if(sock_bind == -1)
	{
		perror("bind error");
		exit(-1);
	}
	if(listen(sockfd,MAXLINK) == -1)
	{
		perror("listen error");
		exit(-1);
	}
return sockfd;
}
void change_passwd(int connfd)
{
	int ret;
	char usr_name[40];
	Account_inf send_inf;
	Account_inf recv_inf;
	bzero(&recv_inf,sizeof(recv_inf));
	bzero(&send_inf,sizeof(send_inf));
	bzero(&res,sizeof(res));
	bzero(&row,sizeof(row));
	fflush(stdout);
	ret = recv(connfd,&recv_inf,sizeof(Account_inf),0);
	if(ret == -1)
	{
		printf("接收密码包失败\n");
		perror("");
	}
	bzero(query,strlen(query));
	bzero(usr_name,strlen(usr_name));
	strcpy(usr_name,recv_inf.usr_name);
	sprintf(query,"select * from login_inf where usr_name='%s'",recv_inf.usr_name);
	mysql_query(&mysql,query);
	res = mysql_store_result(&mysql);
	if(res == NULL)
	{
		send_inf.p_exist = NOTEXIST;
		ret = send(connfd,&send_inf,sizeof(send_inf),0);
		if(ret == -1)
		{
			printf("发送消息失败\n");
		}
	}
	else
	{
		row = mysql_fetch_row(res);
		strcpy(send_inf.question,row[2]);
		send_inf.p_exist = ISEXIST;
//		printf("%d %s\n",send_inf.exist,row[2]);
		ret = send(connfd,&send_inf,sizeof(send_inf),0);
		if(ret == -1)
		{
			printf("发送消息失败\n");
		}
		printf("发送成功\n");
		bzero(&recv_inf,sizeof(recv_inf));
		ret = recv(connfd,&recv_inf,200,0);
		if(ret == -1)
		{	
			printf("接收消息失败\n");
		}
		printf("接收密码成功\n");
		if(strcmp(row[3],recv_inf.answer) == 0)
		{
			bzero(query,strlen(query));
			sprintf(query,"update login_inf set usr_passwd='%s' where usr_name='%s'",recv_inf.usr_passwd,usr_name);
			ret = mysql_query(&mysql,query);
			if(ret == 0)
			{
				bzero(&send_inf,sizeof(send_inf));
				sprintf(send_inf.str,"修改密码成功");
			//	setbuf(stdin,NULL);
				fflush(stdout);
				ret = send(connfd,&send_inf,sizeof(send_inf),0);
				if(ret == -1)
				{
					printf("修改密码信息发送失败\n");
				}
			}
			else
			{
				bzero(&send_inf,sizeof(send_inf));
				sprintf(send_inf.str,"修改密码失败");
				ret = send(connfd,&send_inf,sizeof(send_inf),0);
				if(ret == -1)
				{
					printf("修改密码信息发送失败\n");
				}
				printf("error:%s\n",mysql_error(&mysql));
			}
		}
		else
		{
			bzero(&send_inf,sizeof(send_inf));
			sprintf(send_inf.str,"密保答案错误");
			ret = send(connfd,&send_inf,sizeof(send_inf),0);
			if(ret == -1)
			{
				printf("发送提示消息失败\n");
			}
		}
	}

}
void registed(int connfd,Account_inf usr)
{
	int ret;
	char str[100];
	bzero(str,strlen(str));
	bzero(query,strlen(query));
	sprintf(query,"insert into login_inf values('%s','%s','%s','%s')",usr.usr_name,usr.usr_passwd,usr.question,usr.answer);
	ret = mysql_query(&mysql,query);
	fflush(stdin);
	fflush(stdout);
	if(ret == 0)
	{
		sprintf(str,"注册成功");
		ret = send(connfd,str,sizeof(str),0);
		if(ret == -1)
		{
			printf("注册信息发送失败\n");
		}
	}
	else
	{
		sprintf(str,"注册失败,用户名已存在");
		ret = send(connfd,str,sizeof(str),0);
		if(ret == -1)
		{
			printf("注册信息发送失败\n");
		}
		printf("error:%s\n",mysql_error(&mysql));
	}

}


void *recv_thread(void *p)
{
	int connfd = *(int*)p;
	MSG msg;
	MSG sm;
	int ret;
	int i;
	int location = 0;
	int flag = 0;//NORMAL
	int recv_conn;
	char promp[50];
	char send_usr[20];
	char send_str[220];
	for(i = 0;i < MAXLINK ;i++)
	{
		if(connfd == usrs[i].connfd)
		{
			location = i;
			if(usrs[i].flag == BANNED)
			{
				flag = 1;
			}
			break;
		}
	}
	if(flag == 1)
	{
		pthread_exit(NULL);
	}
	else if(flag == 0)
	{
		while(1)
		{
			bzero(&msg,sizeof(msg));
			bzero(&sm,sizeof(sm));
			bzero(promp,strlen(promp));
			bzero(send_usr,strlen(send_usr));
			bzero(send_str,strlen(send_str));
			fflush(stdout);
			if(usrs[location].flag == BANNED)
			{
				break;
			}	
			ret = recv(connfd,&msg,sizeof(MSG),0);
			if(ret <= 0)
			{
				break;
			}
			printf("%s\n",msg.str);
			fflush(stdin);
			fflush(stdout);
			if(msg.flag == PRIVATE)
			{
				if(usrs[location].flag == BANNED)
				{
					break;
				}
				for(i = 0;i < MAXUSR; i++)
				{
					if(strcmp(usrs[i].usr_name,msg.recv_usr)==0)
					{
						recv_conn = usrs[i].connfd;
						break;
					}
				}
				if(recv_conn == 0)
				{
					sm.flag = PRIVATE;
					sprintf(sm.str,"对方不在线");
					send(connfd,&sm,sizeof(sm),0);//原路返回
				}
				else
				{	
					for(i = 0;i < MAXUSR; i++)
					{
						if(connfd == usrs[i].connfd)
						{
							strcpy(send_usr,usrs[i].usr_name);
						}	
					}
					fflush(stdout);
					sm.flag = PRIVATE;
					sprintf(sm.str,"(私聊信息)%s---->%s",send_usr,msg.str);
					ret = send(recv_conn,&sm,sizeof(sm),0);//发送信息给指定用户
					if(ret == -1)
					{
						printf("服务端转发消息失败\n");
					}
					else
							printf("服务端转发消息成功");
				}

			}
			else if(msg.flag == GROUP)
			{
				if(usrs[location].flag == BANNED)
				{
					break;
				}
				for(i = 0;i < MAXUSR;i ++)
				{
					if(connfd == usrs[i].connfd)
					{
						strcpy(send_usr,usrs[i].usr_name);
					}
				}
				sm.flag = GROUP;
				sprintf(sm.str,"(群聊消息)%s--->%s",send_usr,msg.str);
				for(i=0;i<MAXUSR;i++)
				{
					if(usrs[i].connfd != 0)
					{
						send(usrs[i].connfd,&sm,sizeof(sm),0);
					}
				}
			}
		}
	}
	pthread_exit(NULL);
}

void chat(int connfd)
{
	char flag[5];
	int ret;
	pthread_t tid;
	pthread_create(&tid,NULL,recv_thread,&connfd);
	fflush(stdout);
	while(1)
	{
		bzero(flag,5);
		ret = recv(connfd,flag,5,0);
		if(ret == -1)
		{
			printf("接收失败\n");
		}
		if(atoi(flag) == 1)
		{
			usleep(50);
			break;
		}
		else if(atoi(flag) == 2)
		{
			usleep(50);
			break;
		}
		else if(atoi(flag) == 3)
		{
			pthread_cancel(tid);
			break;
		}
	}

}


int numofOnline()
{
	int i = 0;
	int count = 0;
	for(i=0;i<MAXUSR;i++)
	{
		if(usrs[i].connfd != 0)
		{
			count++;
		}
	}
	return count;
}
void recordnumber(int connfd,Account_inf usr)
{
	int i = 0;
	for(i=0;i<MAXLINK;i++)
	{	
		if(usrs[i].connfd ==  0)
		{
			usrs[i].connfd = connfd;
			strcpy(usrs[i].usr_name,usr.usr_name);
			usrs[i].flag = NORMAL;
			break;
		}
	}
	if(i == MAXLINK)
	{
		printf("登录人数已达上限\n");
	}

}
void serachnum(int connfd)
{
	int num;
	int ret;
	int i;
	MSG sm;
	Onlines usr;
//	char *name[50];
	num = numofOnline();
	fflush(stdin);
	fflush(stdout);
	bzero(&sm,sizeof(sm));
//	bzero(&name,sizeof(name));
	sm.func = SEARCHNUM;
	sprintf(sm.str,"当前在线人数为：%d",num);
	usleep(2);
	ret = send(connfd,&sm,sizeof(sm),0);
	if(ret == -1)
	{
		printf("发送失败\n");		
	}
/*	for(i = 0;i<MAXLINK;i++)
	{
		if(usrs[i].connfd != 0)
		{
			ret = send(connfd,&usrs[i],sizeof(usrs[i]),0);
			if(ret == -1)
			{
				printf("发送失败\n");
			}
		}
	}
	bzero(&usr,sizeof(usr));
	usr.connfd = -1;
	ret = send(connfd,&usrs[i],sizeof(usrs[i]),0);
	if(ret == -1)
	{
		printf("发送end失败\n");
	}*/
	

	printf("发送成功\n");
}
void Account_reset(int connfd)
{
	int i;
	for(i = 0;i  < MAXLINK;i ++)
	{
		if(usrs[i].connfd == connfd)
		{
			usrs[i].connfd = 0;
			bzero(usrs[i].usr_name,strlen(usrs[i].usr_name));
			usrs[i].flag = 0;
			usrs[i].idenity = 0;
			break;
		}
	}
}
void lord_kick(int connfd)
{
	char usrname[40];
	int ret;
	int del_conn = 0;
//	char resp[100];
	int i;
	MSG sm1;
	MSG sm2;
	bzero(usrname,strlen(usrname));
//	bzero(resp,strlen(resp));
	bzero(&sm1,sizeof(sm1));
	bzero(&sm2,sizeof(sm2));
	ret = recv(connfd,usrname,40,0);
	if(ret == -1)
	{
		printf("接收名称失败\n");
	}
	printf("接收名称成功\n");
	for(i = 0;i < MAXLINK; i++)
	{
		if(strcmp(usrs[i].usr_name,usrname)==0)
		{
			del_conn = usrs[i].connfd;
			break;
		}
	}
	printf("%s\n",usrname);
	if(del_conn == 0)
	{
		sm1.prompt = PROM;
		sprintf(sm1.str,"该用户不在聊天室\n");
		ret = send(connfd,&sm1,sizeof(sm1),0);
		if(ret == -1)
		{
			printf("发送失败\n");
		}
	}
	else
	{
		Account_reset(del_conn);
		sm1.prompt = PROM;
		sprintf(sm1.str,"该用户已被踢出聊天室");
		ret = send(connfd,&sm1,sizeof(sm1),0);	
		if(ret == -1)
		{
				printf("发送失败\n");		
		}
		usleep(1);
		sm2.state = KICK;
		sprintf(sm2.str,"您已被群主踢出聊天室");
		ret = send(del_conn,&sm2,sizeof(sm2),0);
		if(ret == -1)
		{
			printf("发送失败\n");
		}
	}

}
void lord_Banned(int connfd)
{
	char usrname[40];
	int ret;
	int recv_conn = 0;
//	int location = 0;
	int i;
	MSG sm1;
	MSG sm2;
	bzero(usrname,strlen(usrname));
	bzero(&sm1,sizeof(sm1));
	bzero(&sm2,sizeof(sm2));
	ret = recv(connfd,usrname,20,0);
	if(ret == -1)
	{
		printf("接收名称失败\n");
	}
	printf("接收名称成功\n");
	for(i = 0;i < MAXLINK; i++)
	{
		if(strcmp(usrs[i].usr_name,usrname)==0)
		{
			recv_conn = usrs[i].connfd;
	//		location = i;
			break;
		}
	}
	printf("%s\n",usrname);
	if(recv_conn == 0)
	{
		sm1.prompt = PROM;
		sprintf(sm1.str,"该用户不在聊天室\n");
		ret = send(connfd,&sm1,sizeof(sm1),0);
		if(ret == -1)
		{
			printf("发送失败\n");
		}
	}
	else
	{
//		usrs[location].flag = BANNED;
		for(i = 0;i < MAXLINK;i ++)
		{
			if(strcmp(usrs[i].usr_name,usrname) == 0)
			{
				recv_conn = usrs[i].connfd;
				usrs[i].flag = BANNED;
			}
		}
		sm1.prompt = PROM;
		sprintf(sm1.str,"该用户已被禁言");
		ret = send(connfd,&sm1,sizeof(sm1),0);	
		if(ret == -1)
		{
				printf("发送失败\n");		
		}
		usleep(1);
		sm2.prompt = PROM;
		sprintf(sm2.str,"您已被群主禁言");//???not send or not recv
		ret = send(recv_conn,&sm2,sizeof(sm2),0);
		if(ret == -1)
		{
			printf("发送失败\n");
		}
	}

}
void lord_liftBan(int connfd)
{
	char usrname[40];
	int ret;
	int recv_conn = 0;
	//int location = 0;
	int i;
	MSG sm1;
	MSG sm2;
	bzero(usrname,strlen(usrname));
	bzero(&sm1,sizeof(sm1));
	bzero(&sm2,sizeof(sm2));
	ret = recv(connfd,usrname,20,0);
	if(ret == -1)
	{
		printf("接收名称失败\n");
	}
	printf("接收名称成功\n");
	for(i = 0;i < MAXLINK; i++)
	{
		if(strcmp(usrs[i].usr_name,usrname)==0)
		{
			recv_conn = usrs[i].connfd;
		//	location = i;
			break;
		}
	}
	printf("%s\n",usrname);
	if(recv_conn == 0)
	{
		sm1.prompt = PROM;
		sprintf(sm1.str,"该用户不在聊天室\n");
		ret = send(connfd,&sm1,sizeof(sm1),0);
		if(ret == -1)
		{
			printf("发送失败\n");
		}
	}
	else
	{
		//usrs[location].flag = NORMAL;
		for(i = 0;i < MAXLINK;i ++)
		{
			if(strcmp(usrs[i].usr_name,usrname) == 0)
			{
				recv_conn = usrs[i].connfd;
				usrs[i].flag = NORMAL;
			}
		}
		sm1.prompt = PROM;
		sprintf(sm1.str,"该用户已被解禁");
		ret = send(connfd,&sm1,sizeof(sm1),0);	
		if(ret == -1)
		{
				printf("发送失败\n");		
		}
		usleep(1);
		sm2.prompt = PROM;
		sprintf(sm2.str,"您已被群主解禁");//???not send or not recv
		ret = send(recv_conn,&sm2,sizeof(sm2),0);
		if(ret == -1)
		{
			printf("发送失败\n");
		}
	}


}
void file_trans(int connfd)
{
	MSG rm;
	MSG sm;
	int ret;
	int i;
	int recv_conn = 0;
	char usrname[40];
	char ver_recv[5];
	bzero(ver_recv,strlen(ver_recv));
	bzero(&sm,sizeof(sm));
	bzero(&rm,sizeof(rm));
	ret = recv(connfd,&rm,sizeof(MSG),0);
	if(ret == -1)
	{
		printf("recv msg error\n");
	}
	for(i = 0;i < MAXLINK; i++)
	{
		if(strcmp(usrs[i].usr_name,rm.recv_usr) == 0)
		{
			recv_conn = usrs[i].connfd;
			break;
		}
	}
	if(recv_conn == 0)
	{
		sprintf(sm.str,"the usr is not int the chatting room\n");
		sm.prompt = PROM;
		ret = send(connfd,&sm,sizeof(sm),0);
		if(ret == -1)
		{
			printf("send promt error\n");
		}
	
	}
	else
	{
		for(i=0;i<MAXLINK;i++)
		{
			if(connfd == usrs[i].connfd)
			{
				strcpy(usrname,usrs[i].usr_name);
				break;
			}
		}
		sm.type = FILE_TRANS;
		sprintf(sm.str,"%s send a file to you.",usrname);
		ret = send(recv_conn,&sm,sizeof(sm),0);
		if(ret == -1)
		{
			printf("send recv_usr file msg error\n");
		}
	/*	ret = recv(recv_conn,ver_recv,5,0);
		if(ret == -1)
		{
			printf("接收确认信息失败\n");
		}
		if(strcmp(ver_recv,"n") == 0)
		{
			bzero(&sm,sizeof(sm));
			sm.type = FILE_SENDS;
			strcpy(sm.ver_recv,ver_recv);
			sprintf(sm.str,"%s拒绝接收文件",rm.recv_usr);
			ret = send(connfd,&sm,sizeof(sm),0);
			if(ret == -1)
			{
				printf("发送失败\n");
			}
		}
		else if(strcmp(ver_recv,"y") == 0)
		{
			bzero(&sm,sizeof(sm));
			sm.type = FILE_SENDS;
			strcpy(sm.ver_recv,ver_recv);
			sprintf(sm.str,"%s同意接收文件",rm.recv_usr);
			ret = send(connfd,&sm,sizeof(sm),0);
			if(ret == -1)
			{
				printf("发送失败\n");
			}*/
			File_inf tmp;
			while(1)
			{
				bzero(&tmp,sizeof(tmp));
				ret = recv(connfd,&tmp,sizeof(File_inf),0);
				if(ret == -1)
				{
					printf("recv file content error\n");
				}
				if(tmp.type == MSG_DONE)
				{
					ret = send(recv_conn,&tmp,sizeof(tmp),0);
					if(ret == -1)
					{
						printf("send MSG_DONE error\n");
					}
					break;
				}
				ret = send(recv_conn,&tmp,sizeof(tmp),0);
				if(ret == -1)
				{
					printf("send msg content error\n");
				}
			}
	//	}

	
	}
}
int chat_module_crowd(int connfd)
{
	int flag;
	char str_flag[5];
	int ret;
	int exit = 0;
	fflush(stdout);
	bzero(str_flag,5);
	ret = recv(connfd,str_flag,5,0);
	if(ret == -1)
	{
		printf("接收聊天模块菜单错误\n");
	}
	flag = atoi(str_flag);
	switch(flag){
			case 1:
					serachnum(connfd);
					break;
			case 2:
					chat(connfd);
					break;
			case 3:
					//文件传输
					file_trans(connfd);
					break;
			case 4:
					exit = 1;
					Account_reset(connfd);
					break;
			default:
					break;

	}
return exit;
}

int chat_module_lord(int connfd)
{
	int flag;
	char str_flag[5];
	int ret;
	int exit = 0;	
	fflush(stdout);
	setbuf(stdin,NULL);
	bzero(str_flag,5);
	ret = recv(connfd,str_flag,5,0);
	if(ret == -1)
	{
		printf("接收聊天模块菜单错误\n");
	}
	flag = atoi(str_flag);
	switch(flag){
			case 1:
					serachnum(connfd);
					break;
			case 2:
					chat(connfd);
					break;
			case 3:
					//文件传输
					file_trans(connfd);
					break;
			case 4:
					//群主踢人
					lord_kick(connfd);
					break;
			case 5:
					//群主禁言
					lord_Banned(connfd);
					break;
			case 6:
					//群主解禁
					lord_liftBan(connfd);
					break;
			case 7:
					exit = 1;
					Account_reset(connfd);
					break;
			default:
					break;

	}

	return exit;
}
void login(int connfd,Account_inf usr)
{
	int ret;
	char str[200];
	Onlines login_inf;
	bzero(str,strlen(str));
	bzero(query,strlen(query));
	bzero(&res,sizeof(res));
	bzero(&login_inf,sizeof(login_inf));
	sprintf(query,"select * from login_inf where usr_name='%s'",usr.usr_name);
	mysql_query(&mysql,query);
	res = mysql_store_result(&mysql);
	fflush(stdin);
	fflush(stdout);
	if(res == NULL)
	{
		sprintf(str,"用户不存在，请先注册");//seg.... error
		ret = send(connfd,str,strlen(str),0);
		if(ret == -1)
		{
			printf("提示信息发送失败\n");
		}
		login_inf.idenity = NOTLOGIN;
		ret = send(connfd,&login_inf,sizeof(login_inf),0);
		if(ret == -1)
		{
			printf("登录信息发送失败\n");	
		}
		close(connfd);//关闭与客户端的连接
	}
	else
	{
		row = mysql_fetch_row(res);
		if(strcmp(usr.usr_passwd,row[1])==0)
		{
			sprintf(str,"登录成功，欢迎%s进入聊天室！",usr.usr_name);
			printf("%s\n",str);
			ret = send(connfd,str,sizeof(str),0);
			if(ret == -1)
			{
				printf("提示信息发送失败\n");
			}
			//接下来进入聊天功能
			recordnumber(connfd,usr);
			if(numofOnline()==1)
			{
				login_inf.idenity = LORD;
				fflush(stdout);
				fflush(stdin);
				usleep(10);
				ret = send(connfd,&login_inf,sizeof(login_inf),0);
				if(ret == -1)
				{
					printf("登录信息发送失败\n");	
				}
				printf("登录信息发送成功\n");
				while(1)
				{
					if(chat_module_lord(connfd) == 1)
							break;
				}
			}
			else
			{
				login_inf.idenity = CROWD;
				usleep(10);
				ret = send(connfd,&login_inf,sizeof(login_inf),0);
				if(ret == -1)
				{
					printf("登录信息发送失败\n");	
				}
				while(1)
				{
					if(chat_module_crowd(connfd) == 1)
							break;
				}

			}
			
		}
		else
		{
			sprintf(str,"密码不正确，登录失败");
			ret = send(connfd,str,strlen(str),0);
			if(ret == -1)
			{
				printf("提示信息发送失败\n");
			}
			login_inf.idenity = NOTLOGIN;
			ret = send(connfd,&login_inf,sizeof(login_inf),0);
			if(ret == -1)
			{
				printf("登录信息发送失败\n");	
			}
			close(connfd);//关闭与客户端的连接
		}
		mysql_free_result(res);
	}
}
void verity(int connfd,Account_inf usr)
{
	int ret;
	bzero(&usr,sizeof(usr));
	fflush(stdout);
	ret = recv(connfd,&usr,sizeof(Account_inf),0);
	if(ret == -1)
	{
		printf("接受验证包失败\n");
	}
	if(usr.flag == 0)//注册模块
	{	
		registed(connfd,usr);	
	}
	else if(usr.flag == 1)//登录模块
	{	
		login(connfd,usr);
	}

}

int recv_menu(int connfd)
{
	char recv_flag[2];
	int ret;
	int flag;
	Account_inf usr;
	fflush(stdout);
	ret = recv(connfd,recv_flag,2,0);
	if(ret == -1)
	{
		return ret;
		//printf("接收菜单失败\n");
	}
	flag = atoi(recv_flag);
	switch(flag){
			case 1:
					verity(connfd,usr);
					break;
			case 2:
					verity(connfd,usr);
					break;
			case 3:
					change_passwd(connfd);//change after
					break;
			case 4:
					printf("用户退出\n");
					close(connfd);
					break;
			default:
					break;
	
	}	
	return ret;
} 
void* service_thread(void* p)//每个客户端对应的服务线程
{
	int ret;
	int connfd = *(int*)p;
	while(1)//首先进入注册登录模块
	{
		ret = recv_menu(connfd);
		if(ret == -1)
		{
			break;
		}
	}
	pthread_exit(NULL);
}
void connect_cli(int sockfd)
{
	struct sockaddr_in caddr;
	int connfd;
	while(1)
	{
		socklen_t len = sizeof(caddr);
		bzero(&caddr,sizeof(caddr));
		connfd = accept(sockfd,(struct sockaddr*)&caddr,&len);
		if(connfd == -1)
		{
			printf("客户端连接出错\n");
			continue;
		}
		int i = 0;
		for(i = 0;i < MAXLINK ;i++)
		{
			if(connfds[i] == 0)
			{
				connfds[i] = connfd;
				pthread_t tid;
				pthread_create(&tid,NULL,service_thread,&connfd);
				break;//控制一次只能生成一个连接
			}	
		}
		if(MAXLINK == i)
		{
			char str[100] = "对不起，聊天室已经满了!";
			send(connfd,str,strlen(str),0);
			close(connfd);
		}
	
	}

}
int main()
{
	int sockfd;
	sockfd = init_socket();
	mysql_init(&mysql);
	if(!mysql_real_connect(&mysql,HOST,USERNAME,PASSWD,DATABASE,3306,NULL,0))
	{
		printf("Connect mysql failed,%s\n",mysql_error(&mysql));
	}
	connect_cli(sockfd);
	mysql_close(&mysql);
	close(sockfd);
	return 0;
}


