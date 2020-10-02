#pragma once

#define COMMAND_BUFFER_MAX 8192
#define TRANSFER_BUFFER_MAX 0x400000
#define FTP_IP "127,0,0,1"
#define TCP_DELIMITER "\r\n"
#define INITIAL_RESPONSE "220 Anonymous FTP server ready."
#define USER_RESPONSE "Guest login ok, send your complete e-mail address as password."
#define PASS_RESPONSE " \r\nWelcome to\r\nSchool of Software\r\nFTP Archives at ftp.ssast.org\r\n \r\nThis site is provided as a public service by School of\r\nSoftware.Use in violation of any applicable laws is strictly\r\nprohibited.We make no guarantees, explicit or implicit, about the\r\ncontents of this site.Use at your own risk.\r\n \r\nGuest login ok, access restrictions apply."
#define SYST_RESPONSE "UNIX Type: L8"
#define TYPE_RESPONSE "Type set to I."
#define PORT_RESPONSE "PORT command successful."
#define RETR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "
int port = 21;
int transferport = 0;
char root[200] = "/tmp";

struct client
{
	int connfd;
	int filefd;
	int transBytes = 0;
	struct sockaddr_in addr;
	int status;
};

enum ClientStatus
{
	PORT_STATUS
};

enum Cmd {
	//USER, PASS, PORT, LIST, PWD, CWD, RETRIEVE, STORE, RESERVED, HALT, QUIT, HELP, PASV, SYST, TYPE, MODE, STRU, NOOP,
	USER, PASS, RETR, STOR, QUIT, ABOR, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, RESERVED
};

enum Cmd s2e(char* s)
{
	if (strcmp(s, "USER") == 0) return USER;
	if (strcmp(s, "PASS") == 0) return PASS;
	if (strcmp(s, "PETR") == 0) return RETR;
	if (strcmp(s, "STOR") == 0) return STOR;
	if (strcmp(s, "QUIT") == 0) return QUIT;
	if (strcmp(s, "ABOR") == 0) return ABOR;
	if (strcmp(s, "SYST") == 0) return SYST;
	if (strcmp(s, "TYPE") == 0) return TYPE;
	if (strcmp(s, "PORT") == 0) return PORT;
	if (strcmp(s, "PASV") == 0) return PASV;
	if (strcmp(s, "MKD") == 0) return MKD;
	if (strcmp(s, "CWD") == 0) return CWD;
	if (strcmp(s, "PWD") == 0) return PWD;
	if (strcmp(s, "LIST") == 0) return LIST;
	if (strcmp(s, "RMD") == 0) return RMD;
	if (strcmp(s, "RNFR") == 0) return RNFR;
	if (strcmp(s, "RNTO") == 0) return RNTO;
}

enum ResCode {
	DATA_CONNECTION_OPEN_TRANSFER_START = 125,

	FILE_STATUS_OKAY = 150,

	HELP_MESSAGE = 214,
	NAME_SYS_TYPE = 215,

	SERVICE_READY = 220,
	SERVICE_CLOSE = 221,

	WELCOME = 230,

	SYST_RESPONSE_CODE = 215,

	TRANSFER_COMPLETE = 226,

	COMMAND_OK = 200,

	PASSIVE_MODE_ON = 227,

	USER_NAME_OK_NEED_PASS = 331,

	FILE_COMMAND_OK = 250,

	PATHNAME_CREATED = 257,

	PERMISSION_DENIED = 500,

	COMMAND_NOT_IMPLEMENTED = 502,

	PARAMETER_NOT_SUPPORTED = 504,

	FILE_UNAVAILABLE = 550,

	RESERVED_CODE = 999

};