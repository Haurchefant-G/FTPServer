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
using namespace std;

#define RETR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "
#define STR(x) #x
#define PASY_RESPONSE "="
int main()
{
	chdir("/mnt/c/Users/12811/wsl/server/test");
	int i = system("rm -rf TtDJfeltyK");
	return 0;

	/*char* info = "12312345";
	int code = 0;
	char* tok = NULL;
	info = strtok_r(info, "\r\n", &tok);*/
	//while (tok[0] != 0)
	//{
	//	printf("%d-%s%s", ++code, info, "\r\n");
	//	info = strtok_r(NULL, "\r\n", &tok);
	//}
	//printf("%d %s%s", ++code, info, "\r\n");
	///*char buf[4000] = RETR_RESPONSE_READY_TO_CONNECT;
	//strcat(buf, info);
	//printf("%s\r\n", buf);*/
	//printf("%s\r\n", RETR_RESPONSE_READY_TO_CONNECT);
	//printf("%s\r\n", info);
	//printf("end");

	//char ip[128];
	///*char a[20] = "", b[20] = "123";
	//strcpy(b, a);
	//printf("((%s))", b);*/
	//char res[2048] = PASY_RESPONSE;
	//chdir("/tmp");
	//int a = mkdir("123/123/123", 0777);
	//printf("%d", a);
	//chdir("/tmp/123/123/123");
	//getcwd(res, sizeof(res));
	//printf("%s", res);
	//int b = 0;
	//printf("%s", res);
	//printf("%s", res);
	//printf("%s", res);
	//getcwd(res, sizeof(res));
	//printf("%s", res);
	//sprintf(&res[strlen(res)], "%s", getServerIP(ip));
	//printf("%s", res);
 //   return 0;
}