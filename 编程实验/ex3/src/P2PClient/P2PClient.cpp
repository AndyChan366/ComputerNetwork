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
#include <sstream>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define DefaultBufLen 2000

typedef struct PacketHead1
{
	unsigned char dataType;
}PacketHead1;

typedef struct PacketHead2a
{
	long long dataSize;
}PacketHead2a;

typedef struct PacketHead2b
{
	char fileName[300];
	long long fileSize;
}PacketHead2b;

SOCKET mySocket = INVALID_SOCKET;
HANDLE myThread = nullptr;
char recvPath[DefaultBufLen];

int getFileSize(char* fileName)
{
	FILE* fp = fopen(fileName, "rb");
	if (!fp) return -1;
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}

int getUniqueName(char* newFileName, char* filePathName)
{
	char result[DefaultBufLen];

	strcpy(result, filePathName);
	strcpy(result + strlen(result), "\\");
	strcpy(result + strlen(result), newFileName);

	FILE* fp = fopen(result, "rb");
	if (!fp)
	{
		strcpy(newFileName, result);
		return strlen(newFileName);
	}
	fclose(fp);

	int count = 1;
	while (true)
	{
		strcpy(result, filePathName);
		strcpy(result + strlen(result), "\\");
		strcpy(result + strlen(result), newFileName);

		char* ptr = strrchr(result, '.');
		char extension[DefaultBufLen];
		strcpy(extension, ptr);

		char number[DefaultBufLen];
		char fileId[DefaultBufLen];
		itoa(count, number, 10);
		strcpy(fileId, "(");
		strcpy(fileId + strlen(fileId), number);
		strcpy(fileId + strlen(fileId), ")");

		strcpy(ptr, fileId);
		strcpy(result + strlen(result), extension);

		fp = fopen(result, "rb");
		if (!fp)
		{
			strcpy(newFileName, result);
			return strlen(newFileName);
		}
		fclose(fp);
		count++;
	}
}

int recvData(SOCKET sock, char* buf, int len)
{
	char recvBuf[DefaultBufLen];
	int result, count;
	count = len;
	while (count)
	{
		if (count < DefaultBufLen)
			result = recv(sock, recvBuf, count, 0);
		else result = recv(sock, recvBuf, DefaultBufLen, 0);
		if (result <= 0) return count;
		strncpy(buf + len - count, recvBuf, result);
		count -= result;
	}
	return len;
}

int recvFile(SOCKET sock, char* fileName, int fileSize, char* path)
{
	char newFileName[DefaultBufLen];
	strcpy(newFileName, fileName);
	getUniqueName(newFileName, path);

	FILE* fp = fopen(newFileName, "wb");
	char recvBuf[DefaultBufLen];
	int result, count;
	count = fileSize;

	while (count)
	{
		if (count < DefaultBufLen)
			result = recv(sock, recvBuf, count, 0);
		else result = recv(sock, recvBuf, DefaultBufLen, 0);
		if (result <= 0) return (fileSize - count);
		fwrite(recvBuf, 1, result, fp);
		count -= result;
	}

	fclose(fp);
	return fileSize;
}

unsigned __stdcall myrecv(void* p)
{
	int result;
	char recvBuf[DefaultBufLen];
	int recvTimeout = 3000;
	setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));

	while (true)
	{
		result = recv(mySocket, recvBuf, sizeof(PacketHead1), 0);

		if (result > 0)
		{
			PacketHead1 tmpph1;
			memcpy(&tmpph1, recvBuf, sizeof(PacketHead1));
			if (tmpph1.dataType == (unsigned char)1)
			{
				result = recv(mySocket, recvBuf, sizeof(PacketHead2a), 0);
				PacketHead2a tmpph2a;
				memcpy(&tmpph2a, recvBuf, sizeof(PacketHead2a));

				char str[DefaultBufLen * 5];
				result = recvData(mySocket, str, (int)tmpph2a.dataSize);
				str[result] = '\0';
				printf("\n< receive message: %s\n> ", str);
			}
			else if (tmpph1.dataType == (unsigned char)2)
			{
				result = recv(mySocket, recvBuf, sizeof(PacketHead2b), 0);
				PacketHead2b tmpph2b;
				memcpy(&tmpph2b, recvBuf, sizeof(PacketHead2b));

				result = recvFile(mySocket, tmpph2b.fileName, (int)tmpph2b.fileSize, recvPath);
				printf("\n< receive file: %s size: %dB\n> ", tmpph2b.fileName, (int)tmpph2b.fileSize);
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

	return 0;
}

int sendFile(SOCKET sock, char* srcFileName)
{
	int result, fileSize, count;
	fileSize = getFileSize(srcFileName);
	count = fileSize;
	FILE* fp = fopen(srcFileName, "rb");
	char sendBuf[DefaultBufLen];

	while (count)
	{
		if (count < DefaultBufLen)
		{
			fread(sendBuf, 1, count, fp);
			result = send(sock, sendBuf, count, 0);
			if (result <= 0) return (fileSize - count);
			count -= result;
		}
		else
		{
			fread(sendBuf, 1, DefaultBufLen, fp);
			result = send(sock, sendBuf, DefaultBufLen, 0);
			if (result <= 0) return (fileSize - count);
			count -= result;
		}
	}
	fclose(fp);
	return (fileSize - count);
}

int sendChatPacket(SOCKET sock, char* chatData)
{
	PacketHead1 tmpph1 = { (unsigned char)1 };
	PacketHead2a tmpph2a = { (long long)strlen(chatData) };

	int result, len, count;
	len = strlen(chatData);
	count = len;
	char sendBuf[DefaultBufLen];

	memcpy(sendBuf, &tmpph1, sizeof(PacketHead1));
	result = send(sock, sendBuf, sizeof(PacketHead1), 0);
	if (result <= 0) return -1;

	memcpy(sendBuf, &tmpph2a, sizeof(PacketHead2a));
	result = send(sock, sendBuf, sizeof(PacketHead2a), 0);
	if (result <= 0) return -1;

	while (count)
	{
		if (count < DefaultBufLen)
		{
			strncpy(sendBuf, chatData + len - count, count);
			result = send(sock, sendBuf, count, 0);
			if (result <= 0) return (len - count);
			count -= result;
		}
		else
		{
			strncpy(sendBuf, chatData + len - count, DefaultBufLen);
			result = send(sock, sendBuf, DefaultBufLen, 0);
			if (result <= 0) return (len - count);
			count -= result;
		}
	}
	return (len - count);
}

int sendFilePacket(SOCKET sock, char* fullFileName)
{
	PacketHead1 tmpph1 = { (unsigned char)2 };
	char* fileName = strrchr(fullFileName, '\\');
	int fileSize = getFileSize(fullFileName);

	PacketHead2b tmpph2b;
	strcpy(tmpph2b.fileName, fileName + 1);
	tmpph2b.fileSize = (long long)fileSize;

	int result;
	char sendBuf[DefaultBufLen];
	memcpy(sendBuf, &tmpph1, sizeof(PacketHead1));
	result = send(sock, sendBuf, sizeof(PacketHead1), 0);
	if (result <= 0) return -1;

	memcpy(sendBuf, &tmpph2b, sizeof(PacketHead2b));
	result = send(sock, sendBuf, sizeof(PacketHead2b), 0);
	if (result <= 0) return -1;

	result = sendFile(sock, fullFileName);
	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Invalid IpAddr/Port\n");
		return 1;
	}

	WSADATA wsaData;
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	getaddrinfo(argv[1], argv[2], &hints, &result);

	mySocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(mySocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	myThread = (HANDLE)_beginthreadex(NULL, 0, &myrecv, NULL, 0, NULL);

	char command[DefaultBufLen];
	char subcommand[DefaultBufLen];
	while (true)
	{
		printf("> ");
		fgets(command, DefaultBufLen, stdin);
		istringstream iss(command);
		iss >> subcommand;
		if (!strcmp(subcommand, "rdir"))
		{
			iss >> subcommand;
			strcpy(recvPath, subcommand);
		}
		else if (!strcmp(subcommand, "chat"))
		{
			iss >> subcommand;
			sendChatPacket(mySocket, subcommand);
		}
		else if (!strcmp(subcommand, "send"))
		{
			iss >> subcommand;
			sendFilePacket(mySocket, subcommand);
		}
		else if (!strcmp(subcommand, "quit"))
		{
			closesocket(mySocket);
			break;
		}
	}

	WaitForSingleObject(myThread, INFINITE);
	CloseHandle(myThread);
	WSACleanup();

	return 0;
}