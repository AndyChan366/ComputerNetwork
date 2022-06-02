#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
    WSADATA wsaData;
    SOCKET Socket = INVALID_SOCKET;
    struct addrinfo* result = NULL, hints;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    int tolen = (int)sizeof(sockaddr_in);

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Resolve the server address and port
    iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (Socket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    freeaddrinfo(result);

    // Send specified buffer
    printf("Enter message to be sent: ");
    scanf("%s", sendbuf);

    iResult = sendto(Socket, sendbuf, (int)strlen(sendbuf) + 1, 0, result->ai_addr, tolen);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(Socket);
        WSACleanup();
        return 1;
    }

    // Receive message from server
    iResult = recvfrom(Socket, recvbuf, recvbuflen, 0, result->ai_addr, &tolen);
    if (iResult > 0)
        printf("\nreceived message:\n%s\n", recvbuf);
    else
        printf("\nrecv failed with error: %d\n", WSAGetLastError());

    // cleanup
    closesocket(Socket);
    WSACleanup();
    system("pause");
    return 0;
}