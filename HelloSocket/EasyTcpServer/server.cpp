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
	//缓冲区
	char szRecv[4096] = {};
	//接收客户数据
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);//先接收报文头
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("客服端已退出，任务结束。\n");
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", _cSock, login->dataLength, login->userName, login->PassWord);
		//忽略判断用户密码是否正确的过程
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		break;
	}

	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", _cSock, logout->dataLength, logout->userName);
		//忽略判断用户密码是否正确的过程
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
	//启动windows socket 2.x环境
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);
	//1、创建套接字
	SOCKET _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2、绑定端口
	sockaddr_in _addr = {};
	_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(4567);
	if (SOCKET_ERROR == bind(_socket, (sockaddr*)&_addr, sizeof(_addr))) {
		printf("错误，绑定用于接受客户端连接的网络端口失败...\n");
	}
	else {
		printf("成功，绑定用于接受客户端连接的网络端口成功...\n");
	}
	//3、监听端口
	if (SOCKET_ERROR == listen(_socket, 5)) {
		printf("错误，监听网络端口失败...\n");
	}
	else {
		printf("成功，监听网络端口成功...\n");
	}
	
	while (true)
	{
		//select BSD套接字
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		//清理集合
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_socket, &fdRead);
		FD_SET(_socket, &fdWrite);
		FD_SET(_socket, &fdExp);

		//现在有空闲的客户端到达，全部加入fdRead
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			FD_SET(g_clients[n], &fdRead);
		}
		/*
		nfds —— 文件描述符加1， 在Windows中这个参数可以写0
		*/
		timeval t = { 0, 0 };
		int ret = select(_socket + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			cout << "select任务结束。\n";
			break;
		}
		//判断描述符（socket）是否在集合中
		if (FD_ISSET(_socket, &fdRead)) {
			FD_CLR(_socket, &fdRead);//现在开始使用，从集合中去除

			//4、接受客户端
			sockaddr_in _caddr = {};//地址
			int caddrlen = sizeof(sockaddr_in);

			SOCKET _csocket = INVALID_SOCKET;//客户端套接字
			_csocket = accept(_socket, (sockaddr*)&_caddr, &caddrlen);
			if (INVALID_SOCKET == _csocket) {
				printf("错误，接受客户端连接失败...\n");
			}
			else {
				for (int n = (int)g_clients.size() - 1; n >= 0; n--) {//和已和服务端连接的客户都说一声，新人来了
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_csocket);
				printf("新客户端加入： IP = %s \n", inet_ntoa(_caddr.sin_addr));
			}
			for (int n = 0; n < (int)fdRead.fd_count; n++) {
				if (-1 == processor(fdRead.fd_array[n])) {//结束后把改句柄从集合中删除
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end())	g_clients.erase(iter);
				}
			}
		}
		/*cout << "服务端做事........\n";*/
	}

	//关闭服务端
	closesocket(_socket);
	WSACleanup();
	getchar();
	return 0;
}
/*
int main() {
	//启动windows socket 2.x环境
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);

	//------------------
	//1、socket 创建套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2、bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _addr = {};
	_addr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(4567);//host to net unsigned short

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_addr, sizeof(_addr))) {
		printf("错误，绑定用于接受客户端连接的网络端口失败.../n");
	}
	else {
		printf("成功，绑定用于接受客户端连接的网络端口成功.../n");
	}
	//3、listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("错误，监听网络端口失败.../n");
	}
	else {
		printf("成功，监听网络端口成功.../n");
	}
	//4、accept 等待接受客户端连接
	sockaddr_in clientAddr = { };
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	while (true)
	{
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock) {
			printf("错误，接受客户端连接失败.../n");
		}
		printf("新客户端加入：IP = %s \n", inet_ntoa(clientAddr.sin_addr));
		//5、 send 向客户端发送一条数据
		char msgBuf[] = "Hello, I'm a server.";
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
	}
	//6、关闭套接字
	closesocket(_sock);
	//------------------

	WSACleanup();
	return 0;
}


*/