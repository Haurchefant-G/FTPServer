#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <dirent.h>
#include "server.h"

//char* getServerIP(char* ip)
//{
//	char hname[128];
//	char temp[200];
//	struct hostent* hent;
//
//	gethostname(hname, sizeof(hname));
//	hent = gethostbyname(hname);
//	for (int i = 0; hent->h_addr_list[i]; i++)
//	{
//		struct in_addr* t = (struct in_addr*)(hent->h_addr_list[i]);
//		int addr = t->s_addr;
//		sprintf(temp, "%d,%d,%d,%d",
//			addr & 0xff,
//			(addr >> 8) & 0xff,
//			(addr >> 16) & 0xff,
//			(addr >> 24) & 0xff);
//	}
//	printf("%s\n", temp);
//	strcpy(ip, temp);
//	return ip;
//}

char* getServerIP(char* ip) {
	//用DNS解析server的IP
	if (ip0 == 0 && ip1 == 0 && ip2 == 0 && ip3 == 0)
	{
		char* dns_address = "8.8.8.8";
		int dns_port = 53;
		int sock;
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			//printf("Error socket(): %s(%d)\n", strerror(errno), errno);
			return ip;
		}
		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(dns_address);
		address.sin_port = htons(dns_port);
		if (connect(sock, (const struct sockaddr*)&address, sizeof(address)) != 0) {
			//printf("Error connect(): %s(%d)\n", strerror(errno), errno);
			return ip;
		}
		memset(&address, 0, sizeof(address));
		socklen_t len = sizeof(address);
		if (getsockname(sock, (struct sockaddr*)&address, &len) != 0) {
			//printf("Error getsockname(): %s(%d)\n", strerror(errno), errno);
			return ip;
		}

		int addr = address.sin_addr.s_addr;
		ip0 = addr & 0xff;
		ip1 = (addr >> 8) & 0xff;
		ip2 = (addr >> 16) & 0xff,
		ip3 = (addr >> 24) & 0xff;
	}
	sprintf(ip, "%d,%d,%d,%d", ip0, ip1, ip2, ip3);
	return ip;
}

int start()
{
	char res[COMMAND_BUFFER_MAX];
	char ip[200];
	getServerIP(ip);
	int listenfd, connfd; //监听socket和连接socket不一样，后者用于数据传输
	struct sockaddr_in serverAddr;

	//创建socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		//printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	//设置本机的ip和port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((uint16_t)port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //监听"0.0.0.0"

	//将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		//printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//开始监听socket
	if (listen(listenfd, SOMAXCONN) == -1)
	{
		//printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//持续监听连接请求
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	while (1)
	{
		//等待client的连接 -- 阻塞函数
		if ((connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &clientAddrSize)) == -1)
		{
			//printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}

		pthread_t t;
		pthread_create(&t, NULL, newClient, (void*)(long)&connfd);
	}

	close(listenfd);
	return 1;
}

void* newClient(void* arg)
{
	pthread_detach(pthread_self());
	struct client c;
	c.status = VISITOR_STATUS;
	c.connfd = ((long*)arg)[0];
	c.acceptfd = -1;
	c.transBytes = 0;
	c.transFileNum = 0;
	c.pathPrefix[0] = '\0';
	c.filename[0] = '\0';
	c.fp = NULL;
	c.fileTrans = (pthread_t)NULL;

	response(c.connfd, SERVICE_READY, INITIAL_RESPONSE);

	char buf[COMMAND_BUFFER_MAX];

	char* cmdArg;

	while (1)
	{
		int n = recv(c.connfd, buf, COMMAND_BUFFER_MAX, 0);
		if (n == 0)
		{
			if (c.status == TRANS_STATUS)
			{
				pthread_cancel(c.fileTrans);
			}
			close(c.connfd);
			pthread_exit(NULL);
			return 0;
		}
		while (n > 0 && (buf[n - 1] == '\r' || buf[n - 1] == '\n'))
		{
			n--;
		}
		buf[n] = '\0';
		char* opt = strtok_r(buf, " ", &cmdArg);
		enum Cmd cmd = s2e(opt);
		switch (cmd)
		{
		case USER:
			if (strcmp(ANONYMOUS_USERNAME, cmdArg) == 0)
			{
				response(c.connfd, USER_NAME_OK_NEED_PASS, USER_RESPONSE);
				c.status = WAIT_PASSWORD_STATUS;
			}
			else
			{
				response(c.connfd, NOT_LOGGED_IN, USER_RESPONSE_FAILED);
			}
			break;
		case PASS:
			response(c.connfd, WELCOME, PASS_RESPONSE);
			c.status = LOGIN_STATUS;
			break;
		case SYST:
			response(c.connfd, SYST_RESPONSE_CODE, SYST_RESPONSE);
			break;
		case TYPE:
			if (!strcmp("I", cmdArg))
			{
				response(c.connfd, COMMAND_OK, TYPE_RESPONSE);
			}
			else
			{
				response(c.connfd, PARAMETER_NOT_SUPPORTED, TYPE_RESPONSE_INVALID);
			}
			break;
		case PORT:
			portCmd(&c, cmdArg);
			break;
		case PASV:
			pasvCmd(&c);
			break;
		case QUIT:
		case ABOR:
			quitCmd(&c);
			pthread_exit(NULL);
			return 0;
		case RETR:
			retrCmd(&c, cmdArg);
			break;
		case STOR:
			storCmd(&c, cmdArg);
			break;
		case MKD:
			mkdCmd(&c, cmdArg);
			break;
		case CWD:
			cwdCmd(&c, cmdArg);
			break;
		case PWD:
			pwdCmd(&c);
			break;
		case LIST:
			listCmd(&c, cmdArg);
			break;
		case RMD:
			rmdCmd(&c, cmdArg);
			break;
		case DELE:
			deleCmd(&c, cmdArg);
			break;
		case RNFR:
			rnfrCmd(&c, cmdArg);
			break;
		case RNTO:
			rntoCmd(&c, cmdArg);
			break;
		default:
			response(c.connfd, 500, "unsupported");
			break;
		}
	}
	pthread_exit(NULL);
}

void response(int connfd, enum ResCode code, char* message)
{
	char buf[COMMAND_BUFFER_MAX];
	buf[0] = '\0';
	char infobuf[COMMAND_BUFFER_MAX];
	char* info = infobuf;
	strcpy(info, message);
	char* tok;
	int i = 0;
	info = strtok_r(info, "\r\n", &tok);
	while (tok[0] != '\0')
	{
		sprintf(&buf[strlen(buf)], "%d-%s%s", code, info, TCP_DELIMITER);
		info = strtok_r(NULL, "\r\n", &tok);
	}
	sprintf(&buf[strlen(buf)], "%d %s%s", code, info, TCP_DELIMITER);
	send(connfd, buf, strlen(buf), 0);
}

int validStatus(struct client* c)
{
	if ((c->status & RENAME_STATUS) == RENAME_STATUS)
	{
		c->status = c->status & CLIENT_STATUS;
		c->filename[0] = '\0';
	}
	if (c->status == LOGIN_STATUS || c->status == PORT_STATUS || c->status == PASV_STATUS)
		return 1;
	else if (c->status == VISITOR_STATUS || c->status == WAIT_PASSWORD_STATUS)
	{
		response(c->connfd, NOT_LOGGED_IN, PERMISSION_DENIED_RESPONSE);
	}
	else if (c->status == TRANS_STATUS)
	{
	}
	return 0;
}

int portCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c->acceptfd != -1)
	{
		close(c->acceptfd);
		c->acceptfd = -1;
	}
	char* h;
	int portArg[6];
	for (int i = 0; i < 6; ++i)
	{
		portArg[i] = atoi(strtok_r(arg, ",", &h));
		arg = NULL;
	}
	char ip[40];
	sprintf(ip, "%d.%d.%d.%d", portArg[0], portArg[1], portArg[2], portArg[3]);
	//in_addr_t ip = (portArg[0] << 24) + (portArg[1] << 16) + (portArg[2] << 8) + portArg[3];
	in_port_t sin_port = portArg[4] * 256 + portArg[5];

	//sockaddr_in portAddress;

	memset(&c->addr, 0, sizeof(struct sockaddr_in));

	c->addr.sin_family = AF_INET;
	c->addr.sin_port = htons(sin_port);
	c->addr.sin_addr.s_addr = inet_addr(ip);

	response(c->connfd, COMMAND_OK, PORT_RESPONSE);
	c->status = PORT_STATUS;
	return 1;
}

//pasv
int pasvCmd(struct client* c)
{
	if (!validStatus(c))
		return 0;
	if (c->acceptfd != -1)
	{
		close(c->acceptfd);
		c->acceptfd = -1;
	}
	//创建socket
	if ((c->acceptfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	struct sockaddr_in serverAddr;
	//设置本机的ip和port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_port = htons((uint16_t)port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //监听"0.0.0.0"

	unsigned short newport;
	//寻找一个未被占用的port
	while (1)
	{
		newport = rand() % 45535 + 20000;
		serverAddr.sin_port = htons(newport);
		//将本机的ip和port与socket绑定
		if (bind(c->acceptfd, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr)) == -1)
		{
			//printf("Port %d cannot be binded", newport);
		}
		else
			break;
	}

	////开始监听socket
	if (listen(c->acceptfd, SOMAXCONN) == -1)
	{
		//printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	char ip[32];
	char res[COMMAND_BUFFER_MAX];
	//= PASY_RESPONSE;
	sprintf(res, PASY_RESPONSE, getServerIP(ip), (newport >> 8) & 0xFF, newport & 0xFF);
	response(c->connfd, PASSIVE_MODE_ON, res);
	c->status = PASV_STATUS;
	return 1;
}

int retrCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c->status == LOGIN_STATUS)
	{
		response(c->connfd, NO_CONNECTION, NO_MODE_SPECIFIED_RESPONSE);
		return 0;
	}
	if (checkPath(processPath(arg)) != IS_FILE)
	{
		response(c->connfd, CANNOT_OPEN_FILE, RETR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	c->fp = NULL;
	c->fp = fopen(arg, "rb");
	// strcpy(c->openfile, arg);
	if (!c->fp)
	{
		//文件不存在
		response(c->connfd, CANNOT_OPEN_FILE, RETR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	fseek(c->fp, 0, SEEK_END);
	int len = ftell(c->fp);
	char res[COMMAND_BUFFER_MAX] = RETR_RESPONSE_READY_TO_CONNECT;
	strcat(res, arg);
	sprintf(&res[strlen(res)], "(%d bytes)", len);
	response(c->connfd, FILE_STATUS_OKAY, res);
	fseek(c->fp, 0, SEEK_SET);
	// fclose(c->fp);

	if (c->status == PORT_STATUS)
	{
		if (connectFileFd(c))
		{
			pthread_create(&c->fileTrans, NULL, sendFile, (void*)c);
		}
		else
		{
			fclose(c->fp);
			return 0;
		}
	}
	else if (c->status == PASV_STATUS)
	{
		c->filefd = accept(c->acceptfd, NULL, NULL);
		pthread_create(&c->fileTrans, NULL, sendFile, (void*)c);
	}
	return 1;
}

void cleanup(void* arg)
{
	struct client* c = (struct client*)arg;
	close(c->filefd);
	if (c->acceptfd != -1)
	{
		close(c->acceptfd);
		c->acceptfd = -1;
	}
	fclose(c->fp);
	c->status = LOGIN_STATUS;
}

void* sendFile(void* arg)
{
	pthread_detach(pthread_self());

	struct client* c = (struct client*)arg;
	pthread_cleanup_push(cleanup, c);
	c->status = TRANS_STATUS;
	char buf[TRANSFER_BUFFER_MAX];
	int sum = 0;
	int n = 0;
	do
	{
		n = fread(buf, sizeof(char), TRANSFER_BUFFER_MAX, c->fp);
		send(c->filefd, buf, n, 0);
		sum += n;
	} while (n > 0);
	close(c->filefd);
	c->transBytes += sum;
	++c->transFileNum;
	if (c->acceptfd != -1)
	{
		close(c->acceptfd);
		c->acceptfd = -1;
	}

	fclose(c->fp);
	c->status = LOGIN_STATUS;

	response(c->connfd, TRANSFER_COMPLETE, RETR_RESPONSE_TRANSEFER_COMPLETE);

	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}

int storCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c->status == LOGIN_STATUS)
	{
		response(c->connfd, NO_CONNECTION, NO_MODE_SPECIFIED_RESPONSE);
		return 0;
	}
	int check = checkPath(processPath(arg));
	if (check != IS_FILE && check != NO_ACCESS)
	{
		//不能访问上层文件或指向文件夹
		response(c->connfd, CANNOT_OPEN_FILE, STOR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	processPath(arg);
	c->fp = NULL;
	c->fp = fopen(arg, "wb");
	if (c->fp == NULL)
	{
		response(c->connfd, CANNOT_OPEN_FILE, STOR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	char res[COMMAND_BUFFER_MAX] = STOR_RESPONSE_READY_TO_CONNECT;
	strcat(res, arg);
	response(c->connfd, FILE_STATUS_OKAY, res);

	if (c->status == PORT_STATUS)
	{
		if (connectFileFd(c))
		{
			pthread_create(&c->fileTrans, NULL, recvFile, (void*)c);
		}
		else
		{
			response(c->connfd, CANNOT_OPEN_FILE, STOR_RESPONSE_CANNOT_OPEN_FILE);
			fclose(c->fp);
			return 0;
		}
	}
	else if (c->status == PASV_STATUS)
	{
		c->filefd = accept(c->acceptfd, NULL, NULL);
		pthread_create(&c->fileTrans, NULL, recvFile, (void*)c);
	}
	return 1;
}

void* recvFile(void* arg)
{
	pthread_detach(pthread_self());

	struct client* c = (struct client*)arg;
	pthread_cleanup_push(cleanup, c);
	c->status = TRANS_STATUS;
	char buf[TRANSFER_BUFFER_MAX];
	int sum = 0;
	int n = 0;
	do
	{
		n = recv(c->filefd, buf, TRANSFER_BUFFER_MAX, 0);
		n = fwrite(buf, sizeof(char), n, c->fp);
		sum += n;
	} while (n > 0);

	close(c->filefd);
	c->transBytes += sum;
	++c->transFileNum;

	fclose(c->fp);
	c->status = LOGIN_STATUS;

	char res[COMMAND_BUFFER_MAX] = STOR_RESPONSE_TRANSEFER_COMPLETE;
	sprintf(&res[strlen(res)], "(%d bytes)", sum);
	response(c->connfd, TRANSFER_COMPLETE, res);

	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}

int connectFileFd(struct client* c)
{
	if ((c->filefd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		//printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	if (connect(c->filefd, (struct sockaddr*)&c->addr, sizeof(c->addr)) < 0)
	{
		//printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	return 1;
}

int quitCmd(struct client* c)
{
	if (c->status == TRANS_STATUS)
	{
		pthread_cancel(c->fileTrans);
	}
	char res[COMMAND_BUFFER_MAX];
	sprintf(res, QUIT_RESPONSE, c->transBytes, c->transFileNum);
	response(c->connfd, SERVICE_CLOSE, res);
	close(c->connfd);
	return 1;
}

char* processPath(char* path)
{
	if (path[0] == '/')
	{
		char pathWithRoot[200];
		strcpy(pathWithRoot, root);
		strcat(pathWithRoot, path);
		strcpy(path, pathWithRoot);
		;
	}
	return path;
}

int checkPath(char* path)
{
	if (strstr(path, "..") != NULL)
		return NO_ACCESS_UPPER_LEVEL;

	char res[COMMAND_BUFFER_MAX];

	struct stat s;

	if (stat(path, &s) == -1)
		return NO_ACCESS;

	if (S_ISDIR(s.st_mode))
		return IS_DIR;
	return IS_FILE;
}

int mkdCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	if (checkPath(processPath(arg)) == IS_DIR)
	{
		sprintf(res, "\"%s\" %s", arg, MKD_RESPONSE_ALREADY_EXIST);
		response(c->connfd, PARAMETER_NOT_SUPPORTED, res);
		return 1;
	}
	if (mkdir(arg, 0777) == 0)
	{
		sprintf(res, "\"%s\" %s", arg, MKD_RESPONSE_OK);
		response(c->connfd, PATHNAME_CREATED, res);
		return 0;
	}
	else
	{
		response(c->connfd, FILE_UNAVAILABLE, MKD_RESPONSE_FAILED);
		return 1;
	}
	return 1;
}

int cwdCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	strcpy(res, arg);
	if (checkPath(processPath(arg)) != IS_DIR)
	{
		sprintf(&res[strlen(res)], ":%s", CWD_RESPONSE_FAILED);
		response(c->connfd, FILE_UNAVAILABLE, res);
		return 1;
	}
	chdir(arg);
	getcwd(res, sizeof(res));
	strcpy(c->pathPrefix, &res[strlen(root)]);
	response(c->connfd, FILE_COMMAND_OK, CWD_RESPONSE_OK);
	return 0;
}

int pwdCmd(struct client* c)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	sprintf(res, "\"%s\"", c->pathPrefix);
	response(c->connfd, PATHNAME_CREATED, res);
	return 0;
}

int listCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c->status == LOGIN_STATUS)
	{
		response(c->connfd, NO_CONNECTION, NO_MODE_SPECIFIED_RESPONSE);
		return 0;
	}
	int check = checkPath(processPath(arg));
	if (strlen(arg) != 0 && (check == NO_ACCESS || check == NO_ACCESS_UPPER_LEVEL))
	{
		response(c->connfd, CANNOT_OPEN_FILE, RETR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}

	char res[COMMAND_BUFFER_MAX];
	sprintf(res, LIST_SYSTEMCMD, arg);
	if (system(res) < 0)
	{
		// 错误
		response(c->connfd, NO_CONNECTION, LIST_RESPONSE_FAILED);
		return 0;
	}

	c->fp = NULL;
	c->fp = fopen(LIST_OUTPUT_FILE, "rb");
	if (!c->fp)
	{
		//文件不存在
		response(c->connfd, NO_CONNECTION, LIST_RESPONSE_FAILED);
		return 0;
	}
	response(c->connfd, FILE_STATUS_OKAY, LIST_RESPONSE_READY_TO_CONNECT);

	if (c->status == PORT_STATUS)
	{
		if (connectFileFd(c))
		{
			pthread_create(&c->fileTrans, NULL, sendFile, (void*)c);
		}
		else
		{
			fclose(c->fp);
			return 0;
		}
	}
	else if (c->status == PASV_STATUS)
	{
		c->filefd = accept(c->acceptfd, NULL, NULL);
		pthread_create(&c->fileTrans, NULL, sendFile, (void*)c);
	}
	return 1;
}

int rmdCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	char dirname[200];
	strcpy(dirname, arg);
	if (checkPath(processPath(arg)) != IS_DIR)
	{
		// 不存在arg指定的文件夹
		sprintf(res, RMD_RESPONSE_FAILED, dirname);
		response(c->connfd, FILE_UNAVAILABLE, res);
		return 1;
	}
	sprintf(res, RMD_SYSTEMCMD, arg);
	if (system(res) != 0)
	{
		// rmd失败
		sprintf(res, RMD_RESPONSE_FAILED, dirname);
		response(c->connfd, FILE_UNAVAILABLE, res);
		return 0;
	}
	// rmd成功
	sprintf(res, RMD_RESPONSE_OK, dirname);
	response(c->connfd, FILE_COMMAND_OK, res);
	return 0;
}

int deleCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	char dirname[200];
	strcpy(dirname, arg);
	if (checkPath(processPath(arg)) != IS_FILE)
	{
		// 不存在arg指定的文件夹
		sprintf(res, DELE_RESPONSE_FAILED, dirname);
		response(c->connfd, FILE_UNAVAILABLE, res);
		return 1;
	}
	sprintf(res, DELE_SYSTEMCMD, arg);
	if (system(res) < 0)
	{
		// rmd失败
		sprintf(res, DELE_RESPONSE_FAILED, dirname);
		response(c->connfd, FILE_UNAVAILABLE, res);
		return 0;
	}
	// rmd成功
	sprintf(res, DELE_RESPONSE_OK, dirname);
	response(c->connfd, FILE_COMMAND_OK, res);
	return 0;
}

int rnfrCmd(struct client* c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	strcpy(c->filename, arg);
	int check = checkPath(processPath(arg));
	if (check == NO_ACCESS || check == NO_ACCESS_UPPER_LEVEL)
	{
		sprintf(res, RNFR_RESPONSE_FAILED, c->filename);
		response(c->connfd, FILE_UNAVAILABLE, res);
		strcpy(c->filename, "\0");
		return 1;
	}
	c->status = c->status | RENAME_STATUS;
	sprintf(&res[strlen(res)], RNFR_RESPONSE_READY_FOR_RENAME, c->filename);
	response(c->connfd, WAIT_FOR_NEW_NAME, res);
	return 0;
}

int rntoCmd(struct client* c, char* arg)
{
	if ((c->status & RENAME_STATUS) != RENAME_STATUS)
	{
		if (!validStatus(c))
			return 0;
		else
		{
			// 上一条指令不为rntoCmd
			response(c->connfd, BAD_SEQUENCE_OF_COMMANDS, RNTO_RESPONSE_WITHOUT_RNFR);
			return 0;
		}
	}
	char res[COMMAND_BUFFER_MAX];
	char newname[200];
	strcpy(newname, arg);
	if (checkPath(processPath(arg)) != NO_ACCESS)
	{
		// 已经存在arg指定的文件或文件夹（名字被占用）
		sprintf(res, RNTO_RESPONSE_ALREADY_EXIST, newname);
		response(c->connfd, FILE_NAME_NOT_ALLOWED, res);
		strcpy(c->filename, "\0");
		return 1;
	}
	sprintf(res, RNTO_SYSTEMCMD, c->filename, newname);
	if (system(res) < 0)
	{
		// rnto失败
		response(c->connfd, FILE_NAME_NOT_ALLOWED, RNTO_RESPONSE_FAILED);
		strcpy(c->filename, "\0");
		return 0;
	}
	// rnto成功
	sprintf(res, RNTO_RESPONSE_OK, c->filename, newname);
	response(c->connfd, FILE_COMMAND_OK, res);
	strcpy(c->filename, "\0");
	return 0;
}

int main(int argc, char* argv[])
{
	char* optstr = "p:r";
	struct option opts[] = {
		{"port", required_argument, NULL, 'p'},
		{"root", required_argument, NULL, 'r'},
	};
	int opt;
	while ((opt = getopt_long_only(argc, argv, optstr, opts, NULL)) != -1)
	{
		switch (opt)
		{
		case 'p':
			//printf("port is %s\n", optarg);
			port = atoi(optarg);
			break;
		case 'r':
			//printf("root is %s\n", optarg);
			strcpy(root, optarg);
			chdir(root);
			break;
		case '?':
		default:
			return 1;
		}
	}
	chdir(root);
	start();
}