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

#define DEFAULT_BUFLEN 1024

SOCKET mySocket = INVALID_SOCKET;
HANDLE myThread = nullptr;

unsigned __stdcall socketRecv(void* p)
{
	int result;
	char* recvBuf = new char[10 * DEFAULT_BUFLEN * DEFAULT_BUFLEN];
	int recvTimeout = 3000;
	setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));

	while (true)
	{
		result = recv(mySocket, recvBuf, DEFAULT_BUFLEN, 0);
		if (result > 0)
		{
			recvBuf[result] = '\0';
			printf("%s\n", recvBuf);
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

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Invalid ip address and port!\n");
		return 0;
	}

	WSADATA wsaData;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	char sendBuf[DEFAULT_BUFLEN];

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	getaddrinfo(argv[1], argv[2], &hints, &result);

	mySocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(mySocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	myThread = (HANDLE)_beginthreadex(NULL, 0, &socketRecv, NULL, 0, NULL);

	while (true)
	{
		if (!fgets(sendBuf, DEFAULT_BUFLEN, stdin))
		{
			shutdown(mySocket, SD_SEND);
			closesocket(mySocket);
			break;
		}

		int sendBufLen = strlen(sendBuf);
		sendBuf[sendBufLen - 1] = '\r';
		sendBuf[sendBufLen] = '\n';
		int result = send(mySocket, sendBuf, sendBufLen + 1, 0);
		if (result == SOCKET_ERROR) break;
	}

	WaitForSingleObject(myThread, INFINITE);
	CloseHandle(myThread);
	WSACleanup();

	return 0;
}