#include "EasyTcpClient.hpp"
#include <thread>

bool g_run = 1;
void CmdThread() {
	while (1) {
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (strcmp(cmdBuf, "exit") == 0) {
			g_run = 0;
			cout << "�˳��߳�" << endl;
			break;
		}
		else cout << "��֧�ָ�����" << endl;
	}
}
int main() {
	const int maxCount = FD_SETSIZE - 1;
	EasyTcpClient* client[maxCount];
	for (int i = 0; i <= maxCount; i++) {
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1", 4567);//192.168.254.1
	}
	thread t(CmdThread);
	t.detach();

	Login login;
	login.userNamer = "StarsDecade";
	login.passwd = "1234";
	while (g_run) {
		for (int i = 0; i <= maxCount; i++) {
			client[i]->SendData(&login);
			client[i]->OnRun();
		}
	}
	for (int i = 0; i <= maxCount; i++)	client[i]->Close();
	cout << "�ͻ����˳�" << endl;
	getchar();
	return 0;
}