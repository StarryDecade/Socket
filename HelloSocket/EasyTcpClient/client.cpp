#include "EasyTcpClient.hpp"
#include<thread>

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >>  cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			printf("��֧�ֵ����\n");
		}
	}
}

int main()
{
	const int cCount = FD_SETSIZE - 1;
	EasyTcpClient* client[cCount];

	for (int n = 0; n < cCount; n++)
	{
		client[n] = new EasyTcpClient();
	}
	for (int n = 0; n < cCount; n++)
	{
		client[n]->Connect("127.0.0.1", 4567);
	}

	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	Login login;
	strncpy(login.userName, "lyd", sizeof(login.userName));
	strncpy(login.PassWord, "lydmm", sizeof(login.PassWord));
	while (g_bRun)
	{
		for (int n = 0; n < cCount; n++)
		{
			client[n]->SendData(&login);
			client[n]->OnRun();
		}

		//printf("����ʱ�䴦������ҵ��..\n");
		//Sleep(1000);
	}

	for (int n = 0; n < cCount; n++)
	{
		client[n]->Close();
	}

	printf("���˳���\n");
	getchar();
	return 0;
}

/*

	������	����	˵��	��Ŀ	�ļ�	��	��ֹ��ʾ״̬
����	C4996	'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.	EasyTcpClient	C:\Four years of university experience\c++\Socket����ͨ�Ż���\HelloSocket\EasyTcpClient\client.cpp	42	


*/