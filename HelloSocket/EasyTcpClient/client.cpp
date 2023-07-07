#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
	#include<windows.h>
	#include<WinSock2.h>
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR		   (-1)

#endif // _WIN32

	

#include<thread>
#include<iostream>
using namespace std;

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;
	short cmd;
};

//DataPackage
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

int processor(SOCKET _cSock) {
	//������
	char szRecv[4096] = {};
	// 5 ���շ��������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("socket = %d ��������Ͽ����ӣ����������\n",int(_cSock));
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* login = (LoginResult*)szRecv;
		printf("socket = %d �յ��������Ϣ��CMD_LOGIN_RESULT,���ݳ��ȣ�%d\n", int(_cSock), login->dataLength);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logout = (LogoutResult*)szRecv;
		printf("socket = %d �յ��������Ϣ��CMD_LOGOUT_RESULT,���ݳ��ȣ�%d\n", int(_cSock), logout->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		printf("socket = %d �յ��������Ϣ��CMD_NEW_USER_JOIN,���ݳ��ȣ�%d\n", int(_cSock), userJoin->dataLength);
	}
	break;
	}
	return 1;
}
bool g_bRun = true;
void cmdThread(SOCKET _socket) {
	while (true)
	{
		char cmdBuf[4096] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit")) {
			cout << "�˳�cmdThread�߳�" << endl;
			g_bRun = false;
			return;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.userName, "StarsDecade");
			strcpy(login.PassWord, "123456");
			int ret = send(_socket, (const char*)&login, sizeof(Login), 0);

		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "StarsDecade");
			send(_socket, (const char*)&logout, sizeof(Logout), 0);
		}
		else {
			cout << "��֧�ָ��������������..." << endl;
		}
	}
}

int main() {
#ifdef _WIN32
	//����windows socket 2.x����
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);
#endif // _WIN32

	
	//1�������׽��֣����ӷ����
	SOCKET _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _socket) {
		printf("socket����ʧ��...\n");
	}
	else {
		printf("socket�����ɹ�...\n");
	}
	//2�����ӷ����
	sockaddr_in _sin = {};
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
	_sin.sin_addr.S_addr = inet_addr("192.168.254.1");
#endif // _WIN32

	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	int ret = connect(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("�������ӷ�����ʧ��...\n");
	}
	else {
		printf("���ӷ������ɹ�...\n");
	}

	//�����߳�
	thread t1(cmdThread, _socket);
	t1.detach();
	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_socket, &fdReads);
		timeval t{ 0, 0 };
		int ret = select(_socket + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select�������1\n");
			break;
		}
		if (FD_ISSET(_socket, &fdReads)) {
			FD_CLR(_socket, &fdReads);

			if (-1 == processor(_socket))
			{
				printf("select�������2\n");
				break;
			}
		}
		/*
		printf("����ʱ�䴦������ҵ��..\n");
		Login login;
		strncpy(login.userName, "lyd", sizeof(login.userName) - 1);
		strncpy(login.PassWord, "lyd", sizeof(login.PassWord) - 1);
		send(_socket, (const char*)&login, sizeof(Login), 0);
		*/
	}
	
	
#ifdef _WIN32
	//4���رտͻ���
	closesocket(_socket);
	WSACleanup();
	printf("�ͷ��˹ر�... \n");
#else
	close(_socket);
#endif // _WIN32

	getchar();
	return 0;
}

/*
int main() {
	//����windows socket 2.x����
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);
	//1������socket
	SOCKET _socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _socket) {
		printf("����SOCKETʧ��...\n");
	}
	else {
		printf("����SOCKET�ɹ�...\n");
	}
	//2�����ӷ�����
	sockaddr_in _saddr = {};
	_saddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_saddr.sin_family = AF_INET;
	_saddr.sin_port = htons(4567);

	int ret = connect(_socket, (sockaddr*)&_saddr, sizeof(sockaddr_in));
	if (INVALID_SOCKET == ret) {
		printf("����SOCKETʧ��...\n");
	}
	else {
		printf("����SOCKET�ɹ�...\n");
	}
	//3�����ܷ�������Ϣ
	char recvBuf[256] = {};
	int nlen = recv(_socket, recvBuf, 256, 0);
	if (nlen > 0) {
		printf("��������:%s \n", recvBuf);
	}
	else
	{
		printf("����ʧ��\n");
	}
	//4���ر��׽���
	closesocket(_socket);
	getchar();
	WSACleanup();
	return 0;
}
*/