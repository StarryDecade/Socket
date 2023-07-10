#ifndef _EasyTcpClient_hpp
#define _EasyTcpClient_hpp

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include "MessageHeader.hpp"
#include <string>
#include <string.h>//内存拷贝memcpy的头文件

class EasyTcpClient {
private:
	SOCKET _socket;
public:
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 102400
#endif 
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};//第二缓冲区 消息缓冲区
	int _lastPos = 0;//消息缓冲区的数据尾部位置
	char _szRecv[RECV_BUFF_SIZE] = {};
	EasyTcpClient();//构造函数——初始化 
	virtual ~EasyTcpClient() {};//系够函数——回收资源
	void InitSocket();//初始化_socket
	int Connect(const char* ip, unsigned short port);//连接服务端
	void Close();//关闭客户端
	int SendData(DataHeader* header);//发送数据
	int RecvData();//接收数据	
	bool IsRun();//判断select是否正在运行
	int OnRun();//用select接受网络消息
	virtual void OnNetMsg(DataHeader* header);
};

inline EasyTcpClient::EasyTcpClient() {
	_socket = INVALID_SOCKET;
}
inline void EasyTcpClient::InitSocket() {
#ifdef _WIN32
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif 
	if (_socket != INVALID_SOCKET) {
		cout << "旧链接socket(" << _socket << ")关闭...." << endl;
		Close();
	}
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET)	cout << "socket 创建失败" << endl;
	else 	cout << "socket 创建成功:" << _socket << endl;
}

inline void EasyTcpClient::Close() {
	if (_socket == INVALID_SOCKET) {
#ifdef _WIN32
		closesocket(_socket);
		//清除Windows socket环境
		WSACleanup();
#else
		close(_socket);//通过句柄（文件描述符）关闭socket
#endif 
		_socket = INVALID_SOCKET;//句柄（文件描述符）——赋为-1
	}
}

inline int EasyTcpClient::Connect(const char* ip, unsigned short port) {
	if (_socket == INVALID_SOCKET)	InitSocket();

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef  _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif 
	int ret = SOCKET_ERROR;
	ret = connect(_socket, (const sockaddr*)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)	cout << _socket << "连接服务端失败..." << endl;
	else cout << _socket << "创建成功" << endl;
	return ret;
}

inline bool EasyTcpClient::IsRun() {
	return _socket == INVALID_SOCKET;
}

inline int EasyTcpClient::OnRun() {
	if (IsRun()) {
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_socket, &fdRead);
		timeval t = { 0, 0 };
		int ret = select(_socket + 1, &fdRead, 0, 0, &t);
		if (ret < 0) {
			cout << _socket << "select 处理网络消息任务结束" << endl;
			Close();
			return 0;
		}
		if (FD_ISSET(_socket, &fdRead)) {
			FD_CLR(_socket, &fdRead);
			if (RecvData()) {
				cout << _socket << "select 执行接受数据任务结束" << endl;
				Close();
				return 1;
			}
		}
		return 0;
	}
	return 0;
}
inline int EasyTcpClient::SendData(DataHeader* header) {
	if (IsRun() && header) {
		return send(_socket, (const char*)header, header->dataLength, 0);
	}
	return INVALID_SOCKET;
}

inline int EasyTcpClient::RecvData() {
	int nlen = (int)recv(_socket, _szRecv, RECV_BUFF_SIZE, 0);
	if (nlen <= 0) {
		cout << _socket << "与服务器断开连接" << endl;
		return -1;
	}
	memcpy(_szMsgBuf + _lastPos, _szRecv, nlen);
	_lastPos += nlen;
	while (_lastPos > sizeof(DataHeader)) {
		DataHeader* header = (DataHeader*)_szMsgBuf;
		if (_lastPos >= header->dataLength) {
			int nsize = _lastPos - header->dataLength;

			memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nsize);
			_lastPos = nsize;
		}
		else break;
	}
	return 0;
}
inline void EasyTcpClient::OnNetMsg(DataHeader* header) {
	switch (header->cmd) {
	case CMD_LOGIN_RESULT: {
		Login* login = (Login*)header;
		cout << login->userNamer << "登录成功！" << endl;
		break;
	}
	case CMD_LOGOUT_RESULT: {
		Logout* logout = (Logout*)header;
		cout << logout->userNamer << "退出成功！" << endl;
		break;
	}
	case CMD_NEW_USER_JOIN: {
		NewUserJoin* newuserjoin = (NewUserJoin*)header;
		cout << newuserjoin->sock << "新用户加入成功";
		break;
	}
	case CMD_ERROR: {
		cout << "接收错误" << endl;
		break;
	}
	default: {
		cout << "收到未定义消息" << endl;
	}
	}
}
#endif 