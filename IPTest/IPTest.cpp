#pragma comment(lib,"comctl32.lib")
#include <cstdio>
#include <string.h>
#include <cstring>
#include <Winsock2.h>

using namespace std;
#define RETR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "

void get_IP_server(char* IP) {
	char hname[128];
	char temp[200];
	const char* t = temp;
	struct hostent* hent;
	int i;

	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);

	for (i = 0; hent->h_addr_list[i]; i++) {
		sprintf_s(temp, "%s", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	}
	printf_s("%s\n", temp);
	strcpy_s(IP, strlen(temp)+1, temp);
}

int main()
{
	char ip[128];
	get_IP_server(ip);
	return 0;
}