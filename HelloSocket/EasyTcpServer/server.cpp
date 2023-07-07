#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<iostream>
#include<vector>
using namespace std;

enum CMD {
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
struct Login: public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT;
	}
	char result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

vector<SOCKET> g_clients;

int processor(SOCKET _cSock) {
	//������
	char szRecv[4096] = {};
	//���տͻ�����
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);//�Ƚ��ձ���ͷ
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("�ͷ������˳������������\n");
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", _cSock, login->dataLength, login->userName, login->PassWord);
		//�����ж��û������Ƿ���ȷ�Ĺ���
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		break;
	}

	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", _cSock, logout->dataLength, logout->userName);
		//�����ж��û������Ƿ���ȷ�Ĺ���
		LogoutResult ret;
		send(_cSock, (char*)&ret, sizeof(ret), 0);
		break;
	}

	default:
	{
		DataHeader header = { 0,CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(header), 0);
	}
		break;
	}
}

int main() {
	//����windows socket 2.x����
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);
	//1�������׽���
	SOCKET _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2���󶨶˿�
	sockaddr_in _addr = {};
	_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(4567);
	if (SOCKET_ERROR == bind(_socket, (sockaddr*)&_addr, sizeof(_addr))) {
		printf("���󣬰����ڽ��ܿͻ������ӵ�����˿�ʧ��...\n");
	}
	else {
		printf("�ɹ��������ڽ��ܿͻ������ӵ�����˿ڳɹ�...\n");
	}
	//3�������˿�
	if (SOCKET_ERROR == listen(_socket, 5)) {
		printf("���󣬼�������˿�ʧ��...\n");
	}
	else {
		printf("�ɹ�����������˿ڳɹ�...\n");
	}
	
	while (true)
	{
		//select BSD�׽���
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		//������
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_socket, &fdRead);
		FD_SET(_socket, &fdWrite);
		FD_SET(_socket, &fdExp);

		//�����п��еĿͻ��˵��ȫ������fdRead
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			FD_SET(g_clients[n], &fdRead);
		}
		/*
		nfds ���� �ļ���������1�� ��Windows�������������д0
		*/
		timeval t = { 0, 0 };
		int ret = select(_socket + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			cout << "select���������\n";
			break;
		}
		//�ж���������socket���Ƿ��ڼ�����
		if (FD_ISSET(_socket, &fdRead)) {
			FD_CLR(_socket, &fdRead);//���ڿ�ʼʹ�ã��Ӽ�����ȥ��

			//4�����ܿͻ���
			sockaddr_in _caddr = {};//��ַ
			int caddrlen = sizeof(sockaddr_in);

			SOCKET _csocket = INVALID_SOCKET;//�ͻ����׽���
			_csocket = accept(_socket, (sockaddr*)&_caddr, &caddrlen);
			if (INVALID_SOCKET == _csocket) {
				printf("���󣬽��ܿͻ�������ʧ��...\n");
			}
			else {
				for (int n = (int)g_clients.size() - 1; n >= 0; n--) {//���Ѻͷ�������ӵĿͻ���˵һ������������
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_csocket);
				printf("�¿ͻ��˼��룺 IP = %s \n", inet_ntoa(_caddr.sin_addr));
			}
			for (int n = 0; n < (int)fdRead.fd_count; n++) {
				if (-1 == processor(fdRead.fd_array[n])) {//������Ѹľ���Ӽ�����ɾ��
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end())	g_clients.erase(iter);
				}
			}
		}
		/*cout << "���������........\n";*/
	}

	//�رշ����
	closesocket(_socket);
	WSACleanup();
	getchar();
	return 0;
}
/*
int main() {
	//����windows socket 2.x����
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);

	//------------------
	//1��socket �����׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2��bind �����ڽ��ܿͻ������ӵ�����˿�
	sockaddr_in _addr = {};
	_addr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(4567);//host to net unsigned short

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_addr, sizeof(_addr))) {
		printf("���󣬰����ڽ��ܿͻ������ӵ�����˿�ʧ��.../n");
	}
	else {
		printf("�ɹ��������ڽ��ܿͻ������ӵ�����˿ڳɹ�.../n");
	}
	//3��listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("���󣬼�������˿�ʧ��.../n");
	}
	else {
		printf("�ɹ�����������˿ڳɹ�.../n");
	}
	//4��accept �ȴ����ܿͻ�������
	sockaddr_in clientAddr = { };
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	while (true)
	{
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock) {
			printf("���󣬽��ܿͻ�������ʧ��.../n");
		}
		printf("�¿ͻ��˼��룺IP = %s \n", inet_ntoa(clientAddr.sin_addr));
		//5�� send ��ͻ��˷���һ������
		char msgBuf[] = "Hello, I'm a server.";
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
	}
	//6���ر��׽���
	closesocket(_sock);
	//------------------

	WSACleanup();
	return 0;
}


*/