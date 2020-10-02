#include <cstdio>
#include <string.h>

#define RETR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "

int main()
{
	char* info = "12312345";
	int code = 0;
	char* tok = NULL;
	info = strtok_r(info, "\r\n", &tok);
	while (tok[0] != 0)
	{
		printf("%d-%s%s", ++code, info, "\r\n");
		info = strtok_r(NULL, "\r\n", &tok);
	}
	printf("%d %s%s", ++code, info, "\r\n");
	/*char buf[4000] = RETR_RESPONSE_READY_TO_CONNECT;
	strcat(buf, info);
	printf("%s\r\n", buf);*/
	printf("%s\r\n", RETR_RESPONSE_READY_TO_CONNECT);
	printf("%s\r\n", info);
	printf("end");
    return 0;
}