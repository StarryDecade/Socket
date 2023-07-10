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
#include <string.h>//�ڴ濽��memcpy��ͷ�ļ�

class EasyTcpClient {
private:
	SOCKET _socket;
public:
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 102400
#endif 
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};//�ڶ������� ��Ϣ������
	int _lastPos = 0;//��Ϣ������������β��λ��
	char _szRecv[RECV_BUFF_SIZE] = {};
	EasyTcpClient();//���캯��������ʼ�� 
	virtual ~EasyTcpClient() {};//ϵ����������������Դ
	void InitSocket();//��ʼ��_socket
	int Connect(const char* ip, unsigned short port);//���ӷ����
	void Close();//�رտͻ���
	int SendData(DataHeader* header);//��������
	int RecvData();//��������	
	bool IsRun();//�ж�select�Ƿ���������
	int OnRun();//��select����������Ϣ
	virtual void OnNetMsg(DataHeader* header);
};

inline EasyTcpClient::EasyTcpClient() {
	_socket = INVALID_SOCKET;
}
inline void EasyTcpClient::InitSocket() {
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif 
	if (_socket != INVALID_SOCKET) {
		cout << "������socket(" << _socket << ")�ر�...." << endl;
		Close();
	}
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET)	cout << "socket ����ʧ��" << endl;
	else 	cout << "socket �����ɹ�:" << _socket << endl;
}

inline void EasyTcpClient::Close() {
	if (_socket == INVALID_SOCKET) {
#ifdef _WIN32
		closesocket(_socket);
		//���Windows socket����
		WSACleanup();
#else
		close(_socket);//ͨ��������ļ����������ر�socket
#endif 
		_socket = INVALID_SOCKET;//������ļ���������������Ϊ-1
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
	if (ret == SOCKET_ERROR)	cout << _socket << "���ӷ����ʧ��..." << endl;
	else cout << _socket << "�����ɹ�" << endl;
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
			cout << _socket << "select ����������Ϣ�������" << endl;
			Close();
			return 0;
		}
		if (FD_ISSET(_socket, &fdRead)) {
			FD_CLR(_socket, &fdRead);
			if (RecvData()) {
				cout << _socket << "select ִ�н��������������" << endl;
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
		cout << _socket << "��������Ͽ�����" << endl;
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
		cout << login->userNamer << "��¼�ɹ���" << endl;
		break;
	}
	case CMD_LOGOUT_RESULT: {
		Logout* logout = (Logout*)header;
		cout << logout->userNamer << "�˳��ɹ���" << endl;
		break;
	}
	case CMD_NEW_USER_JOIN: {
		NewUserJoin* newuserjoin = (NewUserJoin*)header;
		cout << newuserjoin->sock << "���û�����ɹ�";
		break;
	}
	case CMD_ERROR: {
		cout << "���մ���" << endl;
		break;
	}
	default: {
		cout << "�յ�δ������Ϣ" << endl;
	}
	}
}
#endif 