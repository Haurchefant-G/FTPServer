#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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
#include "server.h"


int start()
{
	int listenfd, connfd;		//����socket������socket��һ���������������ݴ���
	struct sockaddr_in serverAddr;
	char sentence[8192];
	int p;
	int len;

	//����socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//���ñ�����ip��port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((uint16_t)port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//����"0.0.0.0"

	//��������ip��port��socket��
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//��ʼ����socket
	if (listen(listenfd, SOMAXCONN) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//����������������
	while (1) {
		//�ȴ�client������ -- ��������
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
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
			break;
		case PASS:
			if (!strcmp("annoymous", cmdArg))
				response(c.connfd, WELCOME, PASS_RESPONSE);
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
			break;
		case QUIT:
		case ABOR:

			break;
		case RETR:
			retrCmd(c, cmdArg);
			break;
		case STOR:

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

int portCmd(struct client& c, char* arg)
{
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


//δ���
int pasvCmd(struct client& c)
{
	//����socket
	if ((c.filefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	sockaddr_in serverAddr;
	//���ñ�����ip��port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_port = htons((uint16_t)port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//����"0.0.0.0"

	unsigned short port;
	//Ѱ��һ��δ��ռ�õ�port
	while (1)
	{
		port = rand() % 45535 + 20000;
		serverAddr.sin_port = htons(port);
		//��������ip��port��socket��
		if (bind(c.filefd, (struct sockaddr*)&serverAddr, sizeof(sockaddr)) == -1) {
			printf("Port %d cannot be binded", port);
		}
		else
			break;
	}

	////��ʼ����socket
	//if (listen(listenfd, SOMAXCONN) == -1) {
	//	printf("Error listen(): %s(%d)\n", strerror(errno), errno);
	//	return 1;
	//}

	accept(c.filefd, NULL, NULL);
}

int retrCmd(struct client& c, char* arg)
{
	if (strncmp(arg, "../", 3) == 0)
	{
		//���ܷ����ϲ��ļ�
		return 0;
	}
	if (c.status == PORT_STATUS)
	{
		FILE* fp = NULL;
		fp = fopen(arg, "rb");
		if (!fp)
		{
			//�ļ�������
		}
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		char res[COMMAND_BUFFER_MAX] = RETR_RESPONSE_READY_TO_CONNECT;
		strcat(res, arg);
		sprintf(&res[strlen(res)], "(%d bytes)", len);
		response(c.connfd, FILE_STATUS_OKAY, res);

		if (connectFileFd(c))
		{
			fseek(fp, 0, SEEK_SET);

			struct fileTrans ft;
			ft.fp = fp;
			ft.c = &c;
			pthread_create(&c.fileTrans, NULL, sendFile, (void*)&ft);

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
			fclose(fp);
			return 0;
		}
		
	}
	else if (c.status == PASV_STATUS)
	{
		return;
	}
}


void cleanup(void* arg)
{
	struct fileTrans* ft = (struct fileTrans*)arg;
	close(ft->c->filefd);
	fclose(ft->fp);
}

void* sendFile(void* arg)
{
	pthread_detach(pthread_self());

	struct fileTrans* ft = (struct fileTrans*)arg;
	pthread_cleanup_push(cleanup, ft);
	char buf[TRANSFER_BUFFER_MAX];
	int sum = 0;
	while (!feof(ft->fp)) {
		int n = fread(buf, sizeof(char), TRANSFER_BUFFER_MAX, ft->fp);
		send(ft->c->filefd, buf, n, 0);
		sum += n;
	}
	close(ft->c->filefd);
	ft->c->transBytes += sum;
	++ft->c->transFileNum;

	fclose(ft->fp);

	response(ft->c->connfd, TRANSFER_COMPLETE, RETR_RESPONSE_TRANSEFER_COMPLETE);

	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}

int storCmd(struct client& c, char* arg)
{
	if (strncmp(arg, "../", 3) == 0)
	{
		//���ܷ����ϲ��ļ�
		return 0;
	}
	if (c.status == PORT_STATUS)
	{
		FILE* fp = NULL;
		fp = fopen(arg, "wb");
		char res[COMMAND_BUFFER_MAX] = STOR_RESPONSE_READY_TO_CONNECT;
		strcat(res, arg);
		response(c.connfd, FILE_STATUS_OKAY, res);

		if (connectFileFd(c))
		{

			struct fileTrans ft;
			ft.fp = fp;
			ft.c = &c;
			pthread_create(&c.fileTrans, NULL, sendFile, (void*)&ft);

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
			fclose(fp);
			return 0;
		}
	}
	else if (c.status == PASV_STATUS)
	{

	}
}

void* recvFile(void* arg)
{
	pthread_detach(pthread_self());

	struct fileTrans* ft = (struct fileTrans*)arg;
	pthread_cleanup_push(cleanup, ft);
	char buf[TRANSFER_BUFFER_MAX];
	int sum = 0;
	while (!feof(ft->fp)) {
		int n = recv(ft->c->filefd, buf, TRANSFER_BUFFER_MAX, 0);
		fwrite(buf, sizeof(char), n, ft->fp);
		sum += n;
	}

	close(ft->c->filefd);
	ft->c->transBytes += sum;
	++ft->c->transFileNum;

	fclose(ft->fp);

	char res[COMMAND_BUFFER_MAX];
	strcat(res, STOR_RESPONSE_TRANSEFER_COMPLETE);
	sprintf(&res[strlen(res)], "(%d bytes)", sum);
	response(ft->c->connfd, TRANSFER_COMPLETE, res);

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