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
#include <csignal>
#include <cstring>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "50500"
#define DEFAULT_SOCKNUM 10

SOCKET ListenSocket;
SOCKET ClientSocket[DEFAULT_SOCKNUM];
HANDLE SocketThread[DEFAULT_SOCKNUM];
int SocketID[DEFAULT_SOCKNUM];
struct sockaddr_in ClientAddr[DEFAULT_SOCKNUM];

void signalHandler(int signum)
{
	for (int i = 0; i < DEFAULT_SOCKNUM; i++)
	{
		if (ClientSocket[i] != INVALID_SOCKET)
		{
			shutdown(ClientSocket[i], SD_SEND);
			closesocket(ClientSocket[i]);
			WaitForSingleObject(SocketThread[i], INFINITE);
			CloseHandle(SocketThread[i]);
		}
	}
	closesocket(ListenSocket);
	WSACleanup();
	exit(0);
}

unsigned __stdcall socketRecv(void* p)
{
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int iResult;
	int id = *(int*)p;

	int recvTimeout = 3000;
	setsockopt(ClientSocket[id], SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));

	while (true)
	{
		iResult = recv(ClientSocket[id], recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			sprintf(sendbuf, "received message: %s", recvbuf);
			printf("%s\n\n", sendbuf);

			for (int i = 0; i < DEFAULT_SOCKNUM; i++)
			{
				if (ClientSocket[i] != INVALID_SOCKET)
					send(ClientSocket[i], sendbuf, (int)strlen(sendbuf) + 1, 0);
			}
		}
		else if (iResult == 0)
			break;
		else if (iResult == SOCKET_ERROR && GetLastError() == WSAENOTSOCK)
			return 1;
	}

	shutdown(ClientSocket[id], SD_SEND);
	closesocket(ClientSocket[id]);
	ClientSocket[id] = INVALID_SOCKET;
	CloseHandle(SocketThread[id]);
	SocketThread[id] = nullptr;
	return 0;
}

int main()
{
	signal(SIGINT, signalHandler);

	WSADATA wsaData;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	int AddrLen = (int)sizeof(sockaddr_in);

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

	ListenSocket = INVALID_SOCKET;
	for (int i = 0; i < DEFAULT_SOCKNUM; i++)
	{
		ClientSocket[i] = INVALID_SOCKET;
		SocketThread[i] = nullptr;
		SocketID[i] = i;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);
	listen(ListenSocket, SOMAXCONN);

	while (true)
	{
		int emptySocketID = -1;
		for (int i = 0; i < DEFAULT_SOCKNUM; i++)
		{
			if (ClientSocket[i] == INVALID_SOCKET)
			{
				emptySocketID = i;
				break;
			}
		}
		if (emptySocketID == -1)
		{
			printf("No Socket is avaliable for now.\n");
			Sleep(3000);
			continue;
		}

		ClientSocket[emptySocketID] = accept(ListenSocket, (struct sockaddr*) & ClientAddr[emptySocketID], &AddrLen);
		SocketThread[emptySocketID] = (HANDLE)_beginthreadex(NULL, 0, &socketRecv, (void*)&(SocketID[emptySocketID]), 0, NULL);
	}

	return 0;
}
