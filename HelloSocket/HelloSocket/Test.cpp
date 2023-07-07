#define WIN32_LEAN_AND_MEAN

#include<windows.h>
#include<WinSock2.h>


int main() {
	//Æô¶¯windows socket 2.x»·¾³
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);


	WSACleanup();
	return 0;
}