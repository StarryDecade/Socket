#define WIN32_LEAN_AND_MEAN

#include<windows.h>
#include<WinSock2.h>
#include<string>
#include<iostream>
using namespace std;

int main() {
	//Æô¶¯windows socket 2.x»·¾³
	WORD var = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(var, &dat);
	string s1 = "abc", s2 = "cba", tmp;
	cout << s1 << endl << s2 << endl << tmp << endl;
	tmp = s1;
	s1 = s2;
	s2 = tmp;
	cout << s1 << endl << s2 << endl << tmp << endl;

	WSACleanup();
	return 0;
}