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


char* getServerIP(char* ip) {
	char hname[128];
	char temp[200];
	struct hostent* hent;

	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);

	for (int i = 0; hent->h_addr_list[i]; i++) {
		//struct in_addr* t = (struct in_addr*)(hent->h_addr_list[i]);
		//int addr = t->s_addr;
		/*ip0 = addr & 0xff;
		ip1 = (addr >> 8) & 0xff;
		ip2 = (addr >> 16) & 0xff;
		ip3 = (addr >> 24) & 0xff;*/
		sprintf(temp, "%s", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	}
	printf("%s\n", temp);
	strcpy(ip, temp);
	return ip;
}

int start()
{
	int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
	struct sockaddr_in serverAddr;
	char sentence[8192];
	int p;
	int len;

	//创建socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//设置本机的ip和port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((uint16_t)port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

	//将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//开始监听socket
	if (listen(listenfd, SOMAXCONN) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//持续监听连接请求
	while (1) {
		//等待client的连接 -- 阻塞函数
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}

		pthread_t t;
		pthread_create(&t, NULL, newClient, (void *)connfd);

		//close(connfd);
	}

	close(listenfd);
}

void * newClient(void * arg)
{
	pthread_detach(pthread_self());
	struct client c;
	c.status = VISITOR_STATUS;
	c.connfd = ((int*)arg)[0];

	response(c.connfd, SERVICE_READY, INITIAL_RESPONSE);

	char buf[COMMAND_BUFFER_MAX];

	char* cmdArg;

	while (1)
	{
		int n = recv(c.connfd, buf, COMMAND_BUFFER_MAX, 0);
		char* opt = strtok_r(buf, " ", &cmdArg);
		Cmd cmd = s2e(opt);
		switch (cmd)
		{
		case USER:
			response(c.connfd, USER_NAME_OK_NEED_PASS, USER_RESPONSE);
			c.status = WAIT_PASSWORD_STATUS;
			break;
		case PASS:
			if (!strcmp("annoymous", cmdArg))
			{
				response(c.connfd, WELCOME, PASS_RESPONSE);
				c.status = LOGIN_STATUS;
			}
			else
			{

			}
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
				//response error code
			}
			break;
		case PORT:
			portCmd(c, cmdArg);
			break;
		case PASV:
			pasvCmd(c);
			break;
		case QUIT:
		case ABOR:
			quitCmd(c);
			pthread_exit(NULL);
			return;
		case RETR:
			retrCmd(c, cmdArg);
			break;
		case STOR:
			storCmd(c, cmdArg);
			break;

		}

	}
	pthread_exit(NULL);
}

void response(int connfd, ResCode code, char* info)
{
	char buf[COMMAND_BUFFER_MAX];
	char* tok;
	info = strtok_r(info, "\r\n", &tok);
	while (tok[0] != 0)
	{
		sprintf(buf, "%d-%s%s", code, info, TCP_DELIMITER);
		info = strtok_r(NULL, "\r\n", &tok);
	}
	sprintf(buf, "%d %s%s", code, info, TCP_DELIMITER);
	send(connfd, buf, strlen(buf), 0);
}

bool validStatus(struct client& c)
{
	if (c.status == LOGIN_STATUS || c.status == PORT_STATUS || c.status == PASV_STATUS)
		return true;
	else if (c.status == VISITOR_STATUS || c.status == WAIT_PASSWORD_STATUS)
	{
		response(c.connfd, NOT_LOGGED_IN, PERMISSION_DENIED_RESPONSE);
	}
	return false;
}

int portCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char * h;
	int portArg[6];
	for (int i = 0; i < 6; ++i) {
		portArg[i] = atoi(strtok_r(arg, ",", &h));
		arg = NULL;
	}
	in_addr_t ip = (portArg[0] << 24) + (portArg[1] << 16) + (portArg[2] << 8) + portArg[3];
	in_port_t port = (portArg[4] << 8) + portArg[5];

	//sockaddr_in portAddress;

	memset(&c.addr, 0, sizeof(sockaddr_in));

	c.addr.sin_family = AF_INET;
	c.addr.sin_port = htons(port);
	c.addr.sin_addr.s_addr = htonl(ip);
	//openDataConnection(portLocation);

	response(c.connfd, COMMAND_OK, PORT_RESPONSE);
	c.status = PORT_STATUS;
}


//未完成
int pasvCmd(struct client& c)
{
	if (!validStatus(c))
		return 0;
	//创建socket
	if ((c.acceptfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	sockaddr_in serverAddr;
	//设置本机的ip和port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_port = htons((uint16_t)port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

	unsigned short port;
	//寻找一个未被占用的port
	while (1)
	{
		port = rand() % 45535 + 20000;
		serverAddr.sin_port = htons(port);
		//将本机的ip和port与socket绑定
		if (bind(c.acceptfd, (struct sockaddr*)&serverAddr, sizeof(sockaddr)) == -1) {
			printf("Port %d cannot be binded", port);
		}
		else
			break;
	}

	////开始监听socket
	if (listen(c.acceptfd, SOMAXCONN) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	char ip[32];
	char res[COMMAND_BUFFER_MAX] = PASY_RESPONSE;
	sprintf(&res[strlen(res)], "s%,%d,%d", getServerIP(ip), (port >> 8) & 0xFF, port & 0xFF);
	response(c.connfd, PASSIVE_MODE_ON, res);

}

int retrCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c.status == LOGIN_STATUS)
	{
		response(c.connfd, NO_CONNECTION, NO_MODE_SPECIFIED_RESPONSE);
		return 0;
	}
	if (checkPath(processPath(arg)) != IS_FILE)
	{
		response(c.connfd, CANNOT_OPEN_FILE, RETR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	c.fp = NULL;
	c.fp = fopen(arg, "rb");
	if (!c.fp)
	{
		//文件不存在
	}
	fseek(c.fp, 0, SEEK_END);
	int len = ftell(c.fp);
	char res[COMMAND_BUFFER_MAX] = RETR_RESPONSE_READY_TO_CONNECT;
	strcat(res, arg);
	sprintf(&res[strlen(res)], "(%d bytes)", len);
	response(c.connfd, FILE_STATUS_OKAY, res);
	fseek(c.fp, 0, SEEK_SET);

	if (c.status == PORT_STATUS)
	{
		if (connectFileFd(c))
		{
			pthread_create(&c.fileTrans, NULL, sendFile, (void*)&c);

			//char buf[TRANSFER_BUFFER_MAX];
			//int sum = 0;
			//while (!feof(fp)) {
			//	int n = fread(buf, sizeof(char), TRANSFER_BUFFER_MAX, fp);
			//	send(c.filefd, buf, n, 0);
			//	sum += n;
			//}
			//close(c.filefd);
			//c.transBytes += sum;
			//++c.transFileNum;

			//response(c.connfd, TRANSFER_COMPLETE, RETR_RESPONSE_TRANSEFER_COMPLETE);
		}
		else
		{
			fclose(c.fp);
			return 0;
		}
		
	}
	else if (c.status == PASV_STATUS)
	{
		c.filefd = accept(c.acceptfd, NULL, NULL);
		pthread_create(&c.fileTrans, NULL, sendFile, (void*)&c);
	}
}


void cleanup(void* arg)
{
	struct client* c = (struct client*)arg;
	close(c->filefd);
	if (c->acceptfd != NULL)
		close(c->acceptfd);
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
	while (!feof(c->fp)) {
		int n = fread(buf, sizeof(char), TRANSFER_BUFFER_MAX, c->fp);
		send(c->filefd, buf, n, 0);
		sum += n;
	}
	close(c->filefd);
	c->transBytes += sum;
	++c->transFileNum;
	if (c->acceptfd != NULL)
		close(c->acceptfd);

	fclose(c->fp);
	c->status = LOGIN_STATUS;

	response(c->connfd, TRANSFER_COMPLETE, RETR_RESPONSE_TRANSEFER_COMPLETE);

	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}

int storCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c.status == LOGIN_STATUS)
	{
		response(c.connfd, NO_CONNECTION, NO_MODE_SPECIFIED_RESPONSE);
		return 0;
	}

	if (checkPath(processPath(arg)) != IS_FILE)
	{
		//不能访问上层文件
		response(c.connfd, CANNOT_OPEN_FILE, STOR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	c.fp = NULL;
	c.fp = fopen(arg, "wb");
	char res[COMMAND_BUFFER_MAX] = STOR_RESPONSE_READY_TO_CONNECT;
	strcat(res, arg);
	response(c.connfd, FILE_STATUS_OKAY, res);

	if (c.status == PORT_STATUS)
	{
		if (connectFileFd(c))
		{

			pthread_create(&c.fileTrans, NULL, sendFile, (void*)&c);

			/*char buf[TRANSFER_BUFFER_MAX];
			int sum = 0;
			while (!feof(fp)) {
				int n = recv(c.filefd, buf, TRANSFER_BUFFER_MAX, 0);
				fwrite(buf, sizeof(char), n, fp);
				sum += n;
			}

			close(c.filefd);
			c.transBytes += sum;
			++c.transFileNum;

			strcat(res, STOR_RESPONSE_TRANSEFER_COMPLETE);
			sprintf(&res[strlen(res)], "(%d bytes)", sum);
			response(c.connfd, TRANSFER_COMPLETE, res);*/
		}
		else
		{
			fclose(c.fp);
			return 0;
		}
	}
	else if (c.status == PASV_STATUS)
	{
		c.filefd = accept(c.acceptfd, NULL, NULL);
		pthread_create(&c.fileTrans, NULL, sendFile, (void*)&c);
	}
}

void* recvFile(void* arg)
{
	pthread_detach(pthread_self());

	struct client* c = (struct client*)arg;
	pthread_cleanup_push(cleanup, c);
	c->status = TRANS_STATUS;
	char buf[TRANSFER_BUFFER_MAX];
	int sum = 0;
	while (!feof(c->fp)) {
		int n = recv(c->filefd, buf, TRANSFER_BUFFER_MAX, 0);
		fwrite(buf, sizeof(char), n, c->fp);
		sum += n;
	}

	close(c->filefd);
	c->transBytes += sum;
	++c->transFileNum;

	fclose(c->fp);
	c->status = LOGIN_STATUS;

	char res[COMMAND_BUFFER_MAX] = STOR_RESPONSE_TRANSEFER_COMPLETE;
	//strcat(res, STOR_RESPONSE_TRANSEFER_COMPLETE);
	sprintf(&res[strlen(res)], "(%d bytes)", sum);
	response(c->connfd, TRANSFER_COMPLETE, res);

	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}

int connectFileFd(struct client& c)
{
	if ((c.filefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	if (connect(c.filefd, (struct sockaddr*)&c.addr, sizeof(c.addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	return 1;
}

int quitCmd(struct client& c)
{
	if (c.status = TRANS_STATUS)
	{
		pthread_cancel(c.fileTrans);
	}
	char res[COMMAND_BUFFER_MAX];
	sprintf(res, QUIT_RESPONSE, c.transBytes, c.transFileNum);
	response(c.connfd, SERVICE_CLOSE, res);
	close(c.connfd);
}

char* processPath(char* path)
{
	if (path[0] == '/')
	{
		char pathWithRoot[200];
		strcpy(pathWithRoot, root);
		strcat(pathWithRoot, path);
		strcpy(path, pathWithRoot);;
	}
	return path;
}

int checkPath(char* path)
{
	if (strstr(path, "../") != NULL)
		return NO_ACCESS;

	//if ((path[0] == '/') && (strstr(path, root) != NULL))

	struct stat s;
	
	if (stat(path, &s) == -1)
		return NO_ACCESS;

	if (s.st_mode & S_IFDIR)
		return IS_DIR;
	return IS_FILE;
}

int mkdCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	if (checkPath(processPath(arg)) == IS_DIR)
	{
		sprintf(res, "\"%s\" %s", arg, MKD_RESPONSE_ALREADY_EXIST);
		response(c.connfd, PARAMETER_NOT_SUPPORTED, res);
		return 1;
	}
	if (mkdir(arg, 0777) == 0)
	{
		sprintf(res, "\"%s\" %s", arg, MKD_RESPONSE_OK);
		response(c.connfd, PARAMETER_NOT_SUPPORTED, res);
		return 0;
	}
	else 
	{
		response(c.connfd, FILE_UNAVAILABLE, MKD_RESPONSE_FAILED);
		return 1;
	}
}

int cwdCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	strcpy(res, arg);
	if (checkPath(processPath(arg)) != IS_DIR)
	{
		sprintf(&res[strlen(res)], ":%s", CWD_RESPONSE_FAILED);
		response(c.connfd, FILE_UNAVAILABLE, res);
		return 1;
	}
	chdir(arg);
	getcwd(res, sizeof(res));
	strcpy(c.pathPrefix, &res[strlen(root)]);
	response(c.connfd, FILE_COMMAND_OK, CWD_RESPONSE_OK);
	return 0;
}

int pwdCmd(struct client& c)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	sprintf(res, "\"%s\"", c.pathPrefix);
	response(c.connfd, PATHNAME_CREATED, res);
	return 0;
}

int listCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	if (c.status == LOGIN_STATUS)
	{
		response(c.connfd, NO_CONNECTION, NO_MODE_SPECIFIED_RESPONSE);
		return 0;
	}
	if (strlen(arg) != 0 && checkPath(processPath(arg)) == NO_ACCESS)
	{
		response(c.connfd, CANNOT_OPEN_FILE, RETR_RESPONSE_CANNOT_OPEN_FILE);
		return 0;
	}
	
	char res[COMMAND_BUFFER_MAX];
	sprintf(res, LIST_SYSTEMCMD, arg);
	if (system(res) < 0)
	{
		// 错误
		return 0;
	}

	c.fp = NULL;
	c.fp = fopen(LIST_OUTPUT_FILE, "rb");
	if (!c.fp)
	{
		//文件不存在
	}
	if (c.status == PORT_STATUS)
	{
		if (connectFileFd(c))
		{

		}
		else
		{
			fclose(c.fp);
			return 0;
		}

	}
	else if (c.status == PASV_STATUS)
	{
		c.filefd = accept(c.acceptfd, NULL, NULL);
	}
	c.status = TRANS_STATUS;
	char buf[TRANSFER_BUFFER_MAX];
	int sum = 0;
	while (!feof(c.fp)) {
		int n = fread(buf, sizeof(char), TRANSFER_BUFFER_MAX, c.fp);
		send(c.filefd, buf, n, 0);
		sum += n;
	}
	close(c.filefd);
	c.transBytes += sum;
	if (c.acceptfd != NULL)
		close(c.acceptfd);
	fclose(c.fp);
	c.status = LOGIN_STATUS;

	response(c.connfd, TRANSFER_COMPLETE, LIST_RESPONSE_TRANSEFER_COMPLETE);
}

int rnfrCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	strcpy(c.rename, arg);
	if (checkPath(processPath(arg)) == NO_ACCESS)
	{
		sprintf(res, RNFR_RESPONSE_FAILED, c.rename);
		response(c.connfd, FILE_UNAVAILABLE, res);
		strcpy(c.rename, "\0");
		return 1;
	}
	sprintf(&res[strlen(res)], RNFR_RESPONSE_READY_FOR_RENAME, c.rename);
	response(c.connfd, WAIT_FOR_NEW_NAME, res);
	return 0;
}

int rntoCmd(struct client& c, char* arg)
{
	if (!validStatus(c))
		return 0;
	char res[COMMAND_BUFFER_MAX];
	char newname[200];
	strcpy(newname, arg);
	if (checkPath(processPath(arg)) == NO_ACCESS)
	{
		sprintf(res, RNTO_RESPONSE_FAILED, newname);
		response(c.connfd, FILE_UNAVAILABLE, res);
		strcpy(c.rename, "\0");
		return 1;
	}
	sprintf(res, RNTO_SYSTEMCMD, c.rename, newname);
	if (system(res) < 0)
	{
		// 错误
		return 0;
	}
	sprintf(res, RNTO_RESPONSE_OK, c.rename, newname);
	response(c.connfd, FILE_COMMAND_OK, res);
	strcpy(c.rename, "\0");
	return 0;
}


int main(int argc, char* argv[])
{
    char* optstr = "p:r:h";
    struct option opts[] = {
        {"port", required_argument, NULL, 'p'},
        {"root", required_argument, NULL, 'r'},
    };
    int opt;
    while ((opt = getopt_long(argc, argv, optstr, opts, NULL)) != -1) {
        switch (opt) {
        case 'p':
			printf("port is %s\n", optarg);
			port = atoi(optarg);
            break;
        case 'r':
			printf("root is %s\n", optarg);
			strcpy(root, optarg);
			chdir(root);
            break;
        case 'h':
            printf("this is help\n");
            break;
        case '?':
        default:
            if (strchr(optstr, optopt) == NULL) {
                fprintf(stderr, "unknown option '-%c'\n", optopt);
            }
            else {
                fprintf(stderr, "option requires an argument '-%c'\n", optopt);
            }
            return 1;
        }
    }
}