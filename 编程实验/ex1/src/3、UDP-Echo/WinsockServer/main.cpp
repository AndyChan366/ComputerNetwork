#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
    WSADATA wsaData;
    int iResult;

    SOCKET Socket = INVALID_SOCKET;

    struct sockaddr_in from;
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int fromlen = (int)sizeof(sockaddr_in);
    int iSendResult;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    char ip_addr[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

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
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (Socket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup UDP socket
    iResult = bind(Socket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(Socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    printf("Server setup already.\n\n");
    while (!_kbhit()) {

        iResult = recvfrom(Socket, recvbuf, recvbuflen, 0, (struct sockaddr*) & from, &fromlen);
        if (iResult > 0) {

            time_t now = time(0);
            char* dt = ctime(&now);
            sprintf(ip_addr, "%d.%d.%d.%d", int(from.sin_addr.S_un.S_un_b.s_b1), int(from.sin_addr.S_un.S_un_b.s_b2),
                int(from.sin_addr.S_un.S_un_b.s_b3), int(from.sin_addr.S_un.S_un_b.s_b4));

            printf("received message: %s\n", recvbuf);
            printf("received time: %s", dt);
            printf("client ip: %s\n", ip_addr);
            printf("client port: %d\n\n", int(from.sin_port));
            sprintf(sendbuf, "time: %smessage: %s\nip: %s\nport: %d", dt, recvbuf, ip_addr, int(from.sin_port));

            // Echo the buffer back to the sender
            iSendResult = sendto(Socket, sendbuf, (int)strlen(sendbuf) + 1, 0, (struct sockaddr*) & from, fromlen);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(Socket);
                WSACleanup();
                return 1;
            }

        }
        else {
            printf("recv failed with error: %d\n\n", WSAGetLastError());
            closesocket(Socket);
            WSACleanup();
            return 1;
        }

    }

    // cleanup
    closesocket(Socket);
    WSACleanup();
    system("pause");
    return 0;
}