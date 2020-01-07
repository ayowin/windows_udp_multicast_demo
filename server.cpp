#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment (lib, "ws2_32.lib")

#define MCASTADDR "233.0.0.1" //本例使用的多播组地址。
#define MCASTPORT 5150 //绑定的本地端口号。
#define BUFSIZE 1024 //接收数据缓冲大小。

int main(int argc, char ** argv)
{
	WSADATA wsd;
	struct sockaddr_in local, remote, from;
	SOCKET sock, sockM;
	TCHAR recvbuf[BUFSIZE];
	/*struct ip_mreq mcast; // Winsock1.0 */
	int len = sizeof(struct sockaddr_in);
	int ret;
	//初始化WinSock2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("WSAStartup() failed/n");

		getchar();
		return -1;
	}
	/*
  　创建一个SOCK_DGRAM类型的SOCKET
  　其中,WSA_FLAG_MULTIPOINT_C_LEAF表示IP多播在控制面层上属于"无根"类型;
  　WSA_FLAG_MULTIPOINT_D_LEAF表示IP多播在数据面层上属于"无根",有关控制面层

	和数据面层有关概念请参阅MSDN说明。
  　*/
	if ((sock = WSASocketW(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("socket failed with:%d/n", WSAGetLastError());
		WSACleanup();

		getchar();
		return -1;
	}
	//将sock绑定到本机某端口上。
	local.sin_family = AF_INET;
	local.sin_port = htons(MCASTPORT);
	local.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("bind failed with:%d /n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();

		getchar();
		return -1;
	}
	//加入多播组
	remote.sin_family = AF_INET;
	remote.sin_port = htons(MCASTPORT);
	inet_pton(AF_INET, MCASTADDR, &(remote.sin_addr.s_addr));

   /* Winsock2.0*/
	if ((sockM = WSAJoinLeaf(sock, (SOCKADDR*)&remote, sizeof(remote), NULL, NULL, NULL, NULL, JL_BOTH)) == INVALID_SOCKET)
	{
		printf("WSAJoinLeaf() failed:%d/n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();

		getchar();
		return -1;
	}

	while (1)
	{
		if ((ret = recvfrom(sock, recvbuf, BUFSIZE, 0, (struct sockaddr*)&from, &len)) == SOCKET_ERROR)
		{
			printf("recvfrom failed with:%d/n", WSAGetLastError());
			closesocket(sockM);
			closesocket(sock);
			WSACleanup();

			getchar();
			return -1;
		}

		if (strcmp(recvbuf, "quit") == 0)
		{
			break;
		}
		else
		{
			recvbuf[ret] = '/0';
			char ip[16] = { 0 };
			inet_ntop(AF_INET, &(from.sin_addr),ip, 16);
			printf("RECV:' %s ' FROM <%s> /n", recvbuf, ip);
		}
	}
	closesocket(sockM);
	closesocket(sock);
	WSACleanup();
	return 0;
}