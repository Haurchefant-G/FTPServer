#pragma once

#define COMMAND_BUFFER_MAX 8192
#define TRANSFER_BUFFER_MAX 0x400000
#define FTP_IP "127,0,0,1"
#define TCP_DELIMITER "\r\n"
#define INITIAL_RESPONSE "Anonymous FTP server ready."
#define ANONYMOUS_USERNAME "anonymous"
#define USER_RESPONSE "Guest login ok, send your complete e-mail address as password."
#define USER_RESPONSE_FAILED "Please use " ANONYMOUS_USERNAME " to login."
#define PASS_RESPONSE " \r\nWelcome to\r\nSchool of Software\r\nFTP Archives\r\n \r\nThis site is provided as a public service by School of\r\nSoftware.Use in violation of any applicable laws is strictly\r\nprohibited.We make no guarantees, explicit or implicit, about the\r\ncontents of this site.Use at your own risk.\r\n \r\nGuest login ok, access restrictions apply."
#define SYST_RESPONSE "UNIX Type: L8"
#define TYPE_RESPONSE "Type set to I."
#define TYPE_RESPONSE_INVALID "Invalid Type."
#define PORT_RESPONSE "PORT command successful."
#define PASY_RESPONSE "Entering Passive Mode (%s,%d,%d)"
#define QUIT_RESPONSE "You have transferred %d bytes in %d files.\r\nThank you for using the FTP service on ftp.ssast.org.\r\nGoodbye."
#define RETR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "
#define RETR_RESPONSE_TRANSEFER_COMPLETE "Transfer complete."
#define RETR_RESPONSE_CANNOT_OPEN_FILE "Cannot open file on server."
#define STOR_RESPONSE_READY_TO_CONNECT "Opening BINARY mode data connection for "
#define STOR_RESPONSE_TRANSEFER_COMPLETE "Transfer complete."
#define STOR_RESPONSE_CANNOT_OPEN_FILE "Cannot open file on server."
#define CWD_RESPONSE_FAILED "No such file or directory."
#define CWD_RESPONSE_OK "Okay."
#define MKD_RESPONSE_ALREADY_EXIST "directory already exists\r\ntaking no action."
#define MKD_RESPONSE_OK "directory created."
#define MKD_RESPONSE_FAILED "Failed to create path."
#define LIST_OUTPUT_FILE "/tmp/listtmp.txt"
#define LIST_SYSTEMCMD "ls -l %s | tail -n+2 > " LIST_OUTPUT_FILE
#define LIST_RESPONSE_READY_TO_CONNECT "Here comes the directory listing."
#define LIST_RESPONSE_TRANSEFER_COMPLETE "List complete."
#define LIST_RESPONSE_FAILED "Can't open data connection."
#define RMD_RESPONSE_FAILED "%s: Directory can't be removed or not found"
#define RMD_SYSTEMCMD "rm -f %s"
#define RMD_RESPONSE_OK "%s: Dictiory is removed successfully."
#define DELE_RESPONSE_FAILED "%s: File can't be deleted or not found"
#define DELE_SYSTEMCMD "rm -f %s"
#define DELE_RESPONSE_OK "%s: File is deleted successfully."
#define RNFR_RESPONSE_FAILED "%s: No such file or directory."
#define RNFR_RESPONSE_READY_FOR_RENAME "%s: File exists, ready for destination name."
#define RNTO_RESPONSE_WITHOUT_RNFR "RNTO must come immediately after RNFR."
#define RNTO_RESPONSE_ALREADY_EXIST "%s: Such file or directory already exsits."
#define RNTO_SYSTEMCMD "mv %s %s"
#define RNTO_RESPONSE_FAILED "Renaming failed."
#define RNTO_RESPONSE_OK "%s renamed to %s successfully."
#define NO_MODE_SPECIFIED_RESPONSE "Can't open data connection."
#define PERMISSION_DENIED_RESPONSE "Not logged in. Permission is denied."
int port = 21;
int transferport = 0;
char root[200] = "/tmp";
//char serverIP[20] = "";
int ip0 = 0, ip1 = 0, ip2 = 0, ip3 = 0;

// client信息
struct client
{
	int connfd;
	int acceptfd;
	int filefd;
	int transBytes;
	int transFileNum;
	FILE* fp;
	struct sockaddr_in addr;
	int status;
	char pathPrefix[200];
	char filename[200];
	char openfile[200];
	pthread_t fileTrans;
};

struct fileTrans
{
	FILE* fp;
	struct client* c;
};

// client状态
enum ClientStatus
{
	VISITOR_STATUS,
	WAIT_PASSWORD_STATUS,
	LOGIN_STATUS,
	PORT_STATUS,
	PASV_STATUS,
	TRANS_STATUS
};

// RNFR相关状态
#define CLIENT_STATUS 0x0f
#define RENAME_STATUS 0x10

// 文件状态
enum StorageAuthority
{
	NO_ACCESS_UPPER_LEVEL,
	NO_ACCESS,
	IS_FILE,
	IS_DIR,
};

// ftp指令
enum Cmd
{
	//USER, PASS, PORT, LIST, PWD, CWD, RETRIEVE, STORE, RESERVED, HALT, QUIT, HELP, PASV, SYST, TYPE, MODE, STRU, NOOP,
	USER,
	PASS,
	RETR,
	STOR,
	QUIT,
	ABOR,
	SYST,
	TYPE,
	PORT,
	PASV,
	MKD,
	CWD,
	PWD,
	LIST,
	RMD,
	DELE,
	RNFR,
	RNTO,
	RESERVED
};

enum Cmd s2e(char* s)
{
	if (strcmp(s, "USER") == 0)
		return USER;
	if (strcmp(s, "PASS") == 0)
		return PASS;
	if (strcmp(s, "RETR") == 0)
		return RETR;
	if (strcmp(s, "STOR") == 0)
		return STOR;
	if (strcmp(s, "QUIT") == 0)
		return QUIT;
	if (strcmp(s, "ABOR") == 0)
		return ABOR;
	if (strcmp(s, "SYST") == 0)
		return SYST;
	if (strcmp(s, "TYPE") == 0)
		return TYPE;
	if (strcmp(s, "PORT") == 0)
		return PORT;
	if (strcmp(s, "PASV") == 0)
		return PASV;
	if (strcmp(s, "MKD") == 0)
		return MKD;
	if (strcmp(s, "CWD") == 0)
		return CWD;
	if (strcmp(s, "PWD") == 0)
		return PWD;
	if (strcmp(s, "LIST") == 0)
		return LIST;
	if (strcmp(s, "RMD") == 0)
		return RMD;
	if (strcmp(s, "DELE") == 0)
		return DELE;
	if (strcmp(s, "RNFR") == 0)
		return RNFR;
	if (strcmp(s, "RNTO") == 0)
		return RNTO;
	return RESERVED;
}

// ftp返回码
enum ResCode
{
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

	WAIT_FOR_NEW_NAME = 350,

	NO_CONNECTION = 425,

	//PERMISSION_DENIED = 500,
	CANNOT_OPEN_FILE = 451,

	COMMAND_NOT_IMPLEMENTED = 502,

	BAD_SEQUENCE_OF_COMMANDS = 503,

	PARAMETER_NOT_SUPPORTED = 504,

	NOT_LOGGED_IN = 530,

	FILE_UNAVAILABLE = 550,

	FILE_NAME_NOT_ALLOWED = 553,

	RESERVED_CODE = 999

};


char* getServerIP(char* ip);
int start();
void* newClient(void* arg);
void response(int connfd, enum ResCode code, char* info);
int validStatus(struct client* c);
int portCmd(struct client* c, char* arg);
int pasvCmd(struct client* c);
int retrCmd(struct client* c, char* arg);
void cleanup(void* arg);
void* sendFile(void* arg);
int storCmd(struct client* c, char* arg);
void* recvFile(void* arg);
int connectFileFd(struct client* c);
int quitCmd(struct client* c);
char* processPath(char* path);
int checkPath(char* path);
int mkdCmd(struct client* c, char* arg);
int cwdCmd(struct client* c, char* arg);
int pwdCmd(struct client* c);
int listCmd(struct client* c, char* arg);
int rmdCmd(struct client* c, char* arg);
int deleCmd(struct client* c, char* arg);
int rnfrCmd(struct client* c, char* arg);
int rntoCmd(struct client* c, char* arg);