#include"socket.h"
#define SERPORT 7100
Account_inf usr;
int sockfd;
//struct sockaddr_in cliaddr;
/*void registerd();
void *recv_thread(void *p);
void chat();
void chat_module();
void login();
void change_passwd();
void init_socket();
void menu();*/

void registerd()
{
	int ret;
	char ser_resp[100];
	bzero(&usr, sizeof(usr));
	bzero(ser_resp,strlen(ser_resp));
	usr.flag = 0;  //用来让服务器辨别这是注册操作
	printf("请输入你的想要的昵称: ");
	setbuf(stdin,NULL);
	scanf("%s",usr.usr_name);
	printf("请输入密码: ");
	setbuf(stdin,NULL);
	scanf("%s",usr.usr_passwd);
	printf("请输入密保问题：");
	setbuf(stdin,NULL);
	gets(usr.question);
	printf("请输入密保问题的答案：");
	setbuf(stdin,NULL);
	gets(usr.answer);
	ret = send(sockfd,&usr,sizeof(Account_inf),0);
	if(ret == -1)
	{
		printf("发送注册包失败\n");
	}
	fflush(stdout);
	ret = recv(sockfd,ser_resp,100,0);
	if(ret == 0)
	{
		printf("接收失败\n");
	}
	printf("%s\n",ser_resp);
}
/*
void *recv_thread(void *p)
{
	char buf[220];
	int ret;
	while(1)
	{
		fflush(stdout);
		bzero(buf,strlen(buf));
		ret = recv(sockfd,buf,220,0);
		if(ret <= 0)
		{
			break;
		}
		printf("%s\n",buf);
	}
	pthread_exit(NULL);
}*/
void private_chat()
{
	int ret;
	char recv_usr[20];
	char str[200];
	MSG msg;
	bzero(recv_usr,strlen(recv_usr));
	bzero(&msg,sizeof(msg));
	msg.flag = PRIVATE;
	printf("请输入你想私聊的用户\n");
	setbuf(stdin,NULL);
	scanf("%s",recv_usr);
	strcpy(msg.recv_usr,recv_usr);
	bzero(str,strlen(str));
	printf("请输入消息内容：\n");
	setbuf(stdin,NULL);
	scanf("%s",str);
	strcpy(msg.str,str);
	ret = send(sockfd,&msg,sizeof(msg),0);
	if(ret == -1)
	{
		printf("发送消息失败\n");
	}
			
}
void group_chat()
{
	int ret;
	char str[200];
	MSG msg;
	bzero(str,strlen(str));
	bzero(&msg,sizeof(msg));
	msg.flag = GROUP;
	printf("请输入消息内容：\n");
	setbuf(stdin,NULL);
	scanf("%s",str);
	strcpy(msg.str,str);
	ret = send(sockfd,&msg,sizeof(msg),0);
	if(ret == -1)
	{
		printf("发送消息失败\n");
	}
//	printf("发送消息成功\n");


}
int chat()//仅仅聊天
{
	char flag[5];
	int exit = 0;
//	pthread_t tid;
//	pthread_create(&tid,NULL,recv_thread,NULL);//接收来自聊天室的信息
	while(1)
	{
		printf("请选择服务\n");
		printf("1.私聊\n");
		printf("2.群聊\n");
		printf("3.退出聊天\n");
		setbuf(stdin,NULL);
		bzero(flag,5);
		scanf("%s",flag);
		send(sockfd,flag,strlen(flag),0);
		if(atoi(flag) == 1)
		{
			private_chat(sockfd);
		}
		else if(atoi(flag) == 2)
		{
			group_chat(sockfd);	
		}
		else if(atoi(flag) == 3)
		{
			exit = 1;
			break;
		}
	
	}
	return exit;

}
void lord_op()
{
	char usrname[40];
	int ret;
	bzero(usrname,strlen(usrname));
	printf("请选择操作的用户名称\n");
	setbuf(stdin,NULL);
	fflush(stdout);
	scanf("%s",usrname);
	ret = send(sockfd,usrname,strlen(usrname),0);
	if(ret == -1)
	{
		printf("群主操作名称发送失败\n");
	}
	printf("群主操作名称发送成功\n");
}
void file_recv()
{
	File_inf inf;
	bzero(&inf,sizeof(inf));
	FILE *fp;
	int ret;
	char path[100];
	size_t len;
	ret = recv(sockfd,&inf,sizeof(File_inf),0);
	if(ret == -1)
	{
		printf("recv msg_filename error\n");		
	}
	if(inf.type == MSG_FILENAME)
	{
		printf("开始接收%s文件\n",inf.content);
		bzero(path,strlen(path));
		sprintf(path,"./file/%s",inf.content);
		fp = fopen(path,"w");
		while(1)
		{
			//	bzero(buf,strlen(buf));
				bzero(&inf,sizeof(inf));
				ret = recv(sockfd,&inf,sizeof(File_inf),0);
				if(ret == -1)
				{
					printf("recv msg_content error");
				}
				if(inf.type == MSG_CONTENT)
				{
					len = fwrite(inf.content,sizeof(char),strlen(inf.content),fp);
					if(len <= 0)
					{
						perror("write error");
					}
				
				}
				else if(inf.type == MSG_DONE)
				{
					printf("接收文件完毕\n");
					fclose(fp);
					break;
				}
			
		}
		
	}
}
/*
void file_send_content()
{
	int ret;
	File_inf send_file;
	FILE *fp;
	size_t len;
	char buf[1024];
	bzero(&send_file,sizeof(send_file));
	printf("input filename\n");
	setbuf(stdin,NULL);
	scanf("%s",send_file.content);
	send_file.type = MSG_FILENAME;
	printf("开始发送...\n");
	ret = send(sockfd,&send_file,sizeof(send_file),0);
	fp = fopen(send_file.content,"r");
	while(1)
	{
		bzero(&send_file,sizeof(send_file));
		send_file.type = MSG_CONTENT;
		memset(buf,0,sizeof(buf));
		len = fread(buf,sizeof(char),1024,fp);
		if(len <= 0)
		{
			if(feof(fp))
			{
				bzero(&send_file,sizeof(send_file));
				send_file.type = MSG_DONE;
				ret = send(sockfd,&send_file,sizeof(send_file),0);
				if(ret == -1)
				{
					printf("msg_done send error\n");
				}
				printf("发送文件完毕!\n");
				fclose(fp);
				break;

			}
			else if(ferror(fp))
			{
				perror("read error");
			}
		}
		strcpy(send_file.content,buf);
		ret = send(sockfd,&send_file,sizeof(send_file),0);
		if(ret == -1)
		{
			printf("msg_content send error\n");
		}
	
	}
	

	
}*/
void *cli_resp(void *p)
{
	MSG msg;
	int ret;
	while(1)
	{
		bzero(&msg,sizeof(msg));
		ret = recv(sockfd,&msg,sizeof(MSG),0);
		if(ret <= 0)
		{
			break;
		}
		if(msg.state == KICK)
		{
			printf("%s\n",msg.str);
			exit(-1);
		}
		else if(msg.flag == PRIVATE || msg.flag == GROUP)
		{
			printf("%s\n",msg.str);
		}
		else if(msg.func == SEARCHNUM)
		{
			printf("%s\n",msg.str);
		/*	Onlines usr;
			while(1)
			{
				bzero(&usr,sizeof(usr));
				ret = recv(sockfd,&usr,sizeof(Onlines),0);
				if(ret <= 0)
				{
					printf("接收失败\n");
				}
				if(usr.connfd == -1)
				{
					break;
				}
				printf("用户:%s\n",usr.usr_name);
			}*/
		}
		else if(msg.prompt == PROM)
		{
			printf("%s\n",msg.str);
		}
	/*	else if(msg.type == FILE_SENDS)
		{
			printf("%s",msg.str);
			if(strcmp(msg.ver_recv,"y") == 0)
			{
				file_send_content();
			}
		
		}*/
		else if(msg.type == FILE_TRANS) 
		{
		//	char ver_recv[5];
			printf("%s\n",msg.str);
		/*	printf("你要接收文件吗y/n\n");
			bzero(ver_recv,strlen(ver_recv));
			scanf("%s",ver_recv);
			ret = send(sockfd,ver_recv,strlen(ver_recv),0);
			if(ret == -1)
			{
				printf("发送确认信息失败\n");
			}*/
	//		if(strcmp(ver_recv,"y") == 0)
	//		{
				file_recv();	
	/*		}
			else if(strcmp(ver_recv,"n") == 0)
			{
				printf("您已拒绝接收\n");
			}
			else 
			{
				printf("非法输入\n");
			}*/
			//first is filename;
			//while  ... content
			//while end if type == MSG_DONE
		}
	}

	pthread_exit(NULL);
}

void file_send()
{
	int ret;
	File_inf send_file;
	FILE *fp;
	size_t len;
	char buf[1024];	
	MSG sm;
	bzero(&sm,sizeof(sm));
	printf("请输入你想把文件传给谁？:\n");
	setbuf(stdin,NULL);
	scanf("%s",sm.recv_usr);
	sm.type = FILE_TRANS;
	ret = send(sockfd,&sm,sizeof(sm),0);
	if(ret == -1)
	{
		printf("send msg file error\n");
	}
	bzero(&send_file,sizeof(send_file));
	printf("输入要传的文件名:\n");
	setbuf(stdin,NULL);
	scanf("%s",send_file.content);
	send_file.type = MSG_FILENAME;
	printf("开始发送...\n");
	ret = send(sockfd,&send_file,sizeof(send_file),0);
	fp = fopen(send_file.content,"r");
	while(1)
	{
		bzero(&send_file,sizeof(send_file));
		send_file.type = MSG_CONTENT;
		memset(buf,0,sizeof(buf));
		len = fread(buf,sizeof(char),1024,fp);
		if(len <= 0)
		{
			if(feof(fp))
			{
				send_file.type = MSG_DONE;
				memset(send_file.content,0,sizeof(send_file.content));
				ret = send(sockfd,&send_file,sizeof(send_file),0);
				if(ret == -1)
				{
					printf("msg_done send error\n");
				}
				printf("发送文件完毕!\n");
				fclose(fp);
				break;

			}
			else if(ferror(fp))
			{
				perror("read error");
			}
		}
		strcpy(send_file.content,buf);
		ret = send(sockfd,&send_file,sizeof(send_file),0);
		if(ret == -1)
		{
			printf("msg_content send error\n");
		}
	
	}
	

	
	
}
int  chat_module(Onlines login_inf)//登录后就可以调用此模块
{
	int flag;
	int ret;
	char str_flag[5];
	int exit = 0;//判断是否退出登陆
	fflush(stdin);
	fflush(stdout);
	pthread_t tid;
	pthread_create(&tid,NULL,cli_resp,NULL);
	if(login_inf.idenity == CROWD)
	{
		printf("请选择服务:\n");
		printf("1:查询在线人数\n");
		printf("2:聊天\n");
		printf("3:文件传输\n");
		printf("4:退出登录\n");
		setbuf(stdin,NULL);
		bzero(str_flag,5);
		scanf("%s",str_flag);
		usleep(10);
		ret = send(sockfd,str_flag,strlen(str_flag),0);
		if(ret == -1)
		{
			printf("发送菜单结果失败\n");
		}
		printf("发送菜单成功\n");
		flag = atoi(str_flag);
		fflush(stdout);
		switch(flag){
				case 1:
					/*	ret = recv(sockfd,ser_resp,50,0);
						printf("接受信息成功\n");
						if(ret == -1)
						{
							printf("接收失败\n");
						}
						printf("%s\n",ser_resp);*/
						break;
				case 2:
						chat();
						break;
				case 3:
						//文件传输
						file_send();
						break;
				case 4:
						pthread_cancel(tid);
						exit = 1;
						break;
				default:
						printf("输入非法\n");
						break;
	
	
		}
		
	}
	else if(login_inf.idenity == LORD)
	{
		printf("请选择服务:\n");
		printf("1:查询在线人数\n");
		printf("2:聊天\n");
		printf("3:文件传输\n");
		printf("4.群主踢人\n");
		printf("5.群主禁言\n");
		printf("6.群主解禁\n");
		printf("7:退出登录\n");
		setbuf(stdin,NULL);
		bzero(str_flag,5);
		scanf("%s",str_flag);
		usleep(10);
		ret = send(sockfd,str_flag,strlen(str_flag),0);
		if(ret == -1)
		{
			printf("发送菜单结果失败\n");
		}
		flag = atoi(str_flag);
	//	usleep(10);
		switch(flag){
				case 1:
					/*	ret = recv(sockfd,ser_resp,50,0);
						printf("jieshou!\n");
						if(ret == -1)
						{
							printf("接收失败\n");
						}
						printf("%s\n",ser_resp);*/
						break;
				case 2:
						chat();
						break;
				case 3:
						//文件传输
						file_send();
						break;
				case 4:
						//群主踢人
						lord_op();
						break;
				case 5:
						//群主禁言
						lord_op();
						break;
				case 6:
						//群主解禁
						lord_op();
						break;
				case 7:
						//退出登录
						exit = 1;
						printf("您已退出聊天室\n");
						pthread_cancel(tid);
						break;
				default:
						printf("输入非法");
						break;
		}
		
	}
	
	return exit;
}
void login()
{
	int ret;
	char ser_resp[200];
	Onlines login_inf;
	bzero(&login_inf,sizeof(login_inf));
	bzero(ser_resp,strlen(ser_resp));
	bzero(&usr,sizeof(usr));
	usr.flag = 1; //用来让服务器辨别这是登录操作
	setbuf(stdin,NULL);
	printf("请输入你的账户名: ");
	scanf("%s",usr.usr_name);
	setbuf(stdin,NULL);
	printf("请输入密码: ");
	scanf("%s",usr.usr_passwd);
	ret = send(sockfd,&usr,sizeof(Account_inf),0);
	if(ret == -1)
	{
		printf("发送登录包失败\n");
	}
	fflush(stdout);
	ret = recv(sockfd,ser_resp,200,0);
	if(ret == -1)
	{
		printf("接收提示信息失败\n");
	}
	printf("%s\n",ser_resp);
	fflush(stdout);
	ret = recv(sockfd,&login_inf,sizeof(Onlines),0);
	if(ret == -1)
	{
		printf("接受登录信息失败\n");
	}
	if(login_inf.idenity == LORD || login_inf.idenity == CROWD)
	{
		while(1)
		{
			if(chat_module(login_inf) == 1)
					break;
		}
	
	}		
	else if(login_inf.idenity == NOTLOGIN)
	{
		printf("登录失败！\n");
	}
}
void change_passwd()
{
	int ret;
//	char ser_resp[100];
	Account_inf send_inf;
	Account_inf recv_inf;
	bzero(&send_inf,sizeof(send_inf));
	bzero(&recv_inf,sizeof(recv_inf));
//	bzero(ser_resp,strlen(ser_resp));
	printf("请输入要修改密码的用户名\n");
	setbuf(stdin,NULL);
	scanf("%s",send_inf.usr_name);
	ret = send(sockfd,&send_inf,sizeof(send_inf),0);
	if(ret == -1)
	{
		printf("发送用户名包失败\n");
		perror("");
	}
	usleep(2);
	ret = recv(sockfd,&recv_inf,sizeof(Account_inf),0);
	if(ret <= 0)
	{
		printf("接收返回信息失败\n");
	}
	if(recv_inf.p_exist == ISEXIST)
	{
		printf("%s\n",recv_inf.question);
		printf("请输入密保问题答案\n");
		bzero(&send_inf,sizeof(send_inf));
		setbuf(stdin,NULL);
		gets(send_inf.answer);
		printf("请输入新密码\n");
		setbuf(stdin,NULL);
		scanf("%s",send_inf.usr_passwd);
		ret = send(sockfd,&send_inf,sizeof(send_inf),0);
		if(ret == -1)
		{
			printf("发送答案:失败\n");
		}
		usleep(2);
		bzero(&recv_inf,sizeof(recv_inf));
		ret = recv(sockfd,&recv_inf,sizeof(Account_inf),0);
		if(ret == -1)
		{
			printf("接受消息失败\n");
		}
		printf("%s\n",recv_inf.str);
	}
	else if(recv_inf.p_exist == NOTEXIST)
	{
		printf("该用户不存在\n");
	}
}
void init_socket()
{
	struct sockaddr_in cliaddr;
	int sock_conn;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		perror("socket error");
		exit(-1);
	}
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(SERPORT);
	cliaddr.sin_addr.s_addr = inet_addr("192.168.100.49");
	sock_conn = connect(sockfd,(struct sockaddr*)&cliaddr,sizeof(struct sockaddr));
	if(sock_conn == -1)
	{
		perror("无法连接到服务器");
		exit(-1);
	}
	printf("成功连接到服务器\n");	
}


void menu()
{
	int flag;
	int ret;
	char str_flag[2];
	printf("请选择服务\n");
	printf("1.注册\n");
	printf("2.登录\n");
	printf("3.修改密码\n");
	printf("4.退出\n");
	fflush(stdin);
	scanf("%s",str_flag);
	flag = atoi(str_flag);
	ret = send(sockfd,str_flag,strlen(str_flag),0);
	if(ret == -1)
	{
		printf("发送菜单失败\n");
	}
	switch(flag){
		case 1:
				registerd();
				break;
		case 2:
				login();
				break;
		case 3:
				change_passwd();
				break;
		case 4:
				close(sockfd);
				exit(0);
				break;
		default:
				printf("错误输入！\n");
				break;
	
	}

}


int main()
{
	init_socket();
	while(1)
	{
		menu();	
	}
	return 0;
}

