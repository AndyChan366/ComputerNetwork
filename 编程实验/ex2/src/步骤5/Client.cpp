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

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "50500"

SOCKET ConnectSocket = INVALID_SOCKET;
HANDLE socketThread = nullptr;

void signalHandler(int signum)
{
    shutdown(ConnectSocket, SD_SEND);
    closesocket(ConnectSocket);
    WaitForSingleObject(socketThread, INFINITE);
    CloseHandle(socketThread);
    WSACleanup();

    exit(1);
}

unsigned __stdcall socketRecv(void* p)
{
    int recvTimeout = 3000;
    setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
    
    char recvBuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;

    while (true)
    {
        iResult = recv(ConnectSocket, recvBuf, recvbuflen, 0);
        if (iResult > 0)

        {
            recvBuf[iResult] = '\0';
            printf("\r%s", recvBuf);
        }
        else if (iResult == 0)
            break;
        else if (iResult == SOCKET_ERROR && GetLastError() == WSAENOTSOCK)
            return 1;
    }
    shutdown(ConnectSocket, SD_SEND);
    closesocket(ConnectSocket);
    return 0;
}

int main()
{
    signal(SIGINT, signalHandler);

    WSADATA wsaData;
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    char sendbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    getaddrinfo("103.26.79.35", DEFAULT_PORT, &hints, &result);

    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    socketThread = (HANDLE)_beginthreadex(NULL, 0, &socketRecv, NULL, 0, NULL);

    while (true)
    {
        printf(">>> ");
        scanf("%s", sendbuf);
        if (!strcmp(sendbuf, "quit"))
            break;
        send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
    }

    shutdown(ConnectSocket, SD_SEND);
    closesocket(ConnectSocket);
    WaitForSingleObject(socketThread, INFINITE);
    CloseHandle(socketThread);
    WSACleanup();
    system("pause");
    return 0;
}