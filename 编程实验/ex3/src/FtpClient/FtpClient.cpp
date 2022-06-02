#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <csignal>

#pragma comment(lib, "Ws2_32.lib")

#define DefaultFtpPort "21"
#define DefaultBufLen 2000

SOCKET mySocket = INVALID_SOCKET;
HANDLE myThread = nullptr;

int rdy;
char msg[DefaultBufLen];

unsigned __stdcall socketRecv(void* p)
{
	int result;
	char* recvBuf = new char[DefaultBufLen];
	int recvTimeout = 3000;
	setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));

	while (true)
	{
		result = recv(mySocket, recvBuf, DefaultBufLen, 0);
		if (result > 0)
		{
			recvBuf[result] = '\0';
			printf("%s\n", recvBuf);
			if (!strncmp(recvBuf, "227", 3))
			{
				rdy = 1;
				strcpy(msg, recvBuf);
			}
		}
		else if (result == 0)
		{
			shutdown(mySocket, SD_SEND);
			closesocket(mySocket);
			break;
		}
		else if (result == SOCKET_ERROR && GetLastError() == WSAENOTSOCK) break;	
	}
	delete[] recvBuf;
	return 0;
}

void strProcess(int* ipAddr, char* ip, char* port)
{
	char tmp[DefaultBufLen] = "";

	itoa(ipAddr[0], tmp, 10);
	strcpy(ip, tmp);
	strcpy(ip + strlen(ip), ".");

	itoa(ipAddr[1], tmp, 10);
	strcpy(ip + strlen(ip), tmp);
	strcpy(ip + strlen(ip), ".");

	itoa(ipAddr[2], tmp, 10);
	strcpy(ip + strlen(ip), tmp);
	strcpy(ip + strlen(ip), ".");

	itoa(ipAddr[3], tmp, 10);
	strcpy(ip + strlen(ip), tmp);

	itoa(ipAddr[4] * 256 + ipAddr[5], port, 10);
}

void saveFile(SOCKET socket, char* fileName)
{
	FILE* fp = fopen(fileName, "wb");

	int result;
	char recvBuf[DefaultBufLen];
	int recvTimeout = 3000;
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
	
	while (true)
	{
		result = recv(socket, recvBuf, DefaultBufLen, 0);
		if (result > 0) fwrite(recvBuf, 1, result, fp);
		else
		{
			shutdown(socket, SD_SEND);
			closesocket(socket);
			break;
		}
	}
	fclose(fp);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	getaddrinfo(argv[1], DefaultFtpPort, &hints, &result);

	mySocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(mySocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	myThread = (HANDLE)_beginthreadex(NULL, 0, &socketRecv, NULL, 0, NULL);

	char msg1[] = "user net\r\n";
	char msg2[] = "pass 123456\r\n";
	char msg3[] = "type I\r\n";
	char msg4[] = "pasv\r\n";
	char msg5[] = "retr ";
	char msg6[] = "quit\r\n";

	send(mySocket, msg1, strlen(msg1), 0);
	send(mySocket, msg2, strlen(msg2), 0);
	send(mySocket, msg3, strlen(msg3), 0);
	send(mySocket, msg4, strlen(msg4), 0);

	char sendBuf[DefaultBufLen];
	strcpy(sendBuf, msg5);
	strcpy(sendBuf + strlen(sendBuf), argv[2]);
	strcpy(sendBuf + strlen(sendBuf), "\r\n");

	send(mySocket, sendBuf, strlen(sendBuf), 0);

	while (!rdy) continue;
	int rdy = 0;

	SOCKET dataSocket = INVALID_SOCKET;
	char* ptr1 = strchr(msg, '(');
	int ipAddr[6];
	sscanf(ptr1, "(%d,%d,%d,%d,%d,%d)", &ipAddr[0], &ipAddr[1],
		&ipAddr[2], &ipAddr[3], &ipAddr[4], &ipAddr[5]);
	char ip[DefaultBufLen] = "";
	char port[DefaultBufLen] = "";
	strProcess(ipAddr, ip, port);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	getaddrinfo(ip, port, &hints, &result);

	dataSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(dataSocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	saveFile(dataSocket, argv[3]);

	send(mySocket, msg6, strlen(msg6), 0);
	WaitForSingleObject(myThread, INFINITE);
	CloseHandle(myThread);
	WSACleanup();

	return 0;
}