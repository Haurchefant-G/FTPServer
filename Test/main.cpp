#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
using namespace std;

#define RETR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "
#define STR(x) #x
#define PASY_RESPONSE "="

void get_IP_server(char* IP) {
	char hname[128];
	char temp[200];
	struct hostent* hent;
	int i;

	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);

	for (i = 0; hent->h_addr_list[i]; i++) {
		struct in_addr* t = (struct in_addr*)(hent->h_addr_list[i]);
		int addr = t->s_addr;
		printf("%d,%d,%d,%d\n", addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);
		sprintf(temp, "%s", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	}
	printf("%s\n", temp);
	strcpy(IP, temp);
}

char * getServerIP(char * ip) {
	char hname[128];
	char temp[200];
	struct hostent* hent;

	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);

	for (int i = 0; hent->h_addr_list[i]; i++) {
		struct in_addr* t = (struct in_addr*)(hent->h_addr_list[i]);
		int addr = t->s_addr;
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

int main()
{
	char* info = "12312345";
	int code = 0;
	char* tok = NULL;
	info = strtok_r(info, " ", &tok);
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