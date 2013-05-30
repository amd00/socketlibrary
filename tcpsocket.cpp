
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "tcpsocket.h"

////////////////////////////////////////////////////////////////////////////////////

TcpSocket::TcpSocket(const char* ip_addr, int port) : Socket()
{
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(!inet_aton(ip_addr ? ip_addr : "127.0.0.1", &addr.sin_addr))
		fprintf(stderr, "Неверный адрес \"%s\"\n", ip_addr);
}

TcpSocket::TcpSocket(int fd) : Socket(fd)
{
	memset(&addr, 0, sizeof(addr));
	socklen_t addr_len = sizeof(addr);
	if(getpeername(fd, (sockaddr*)&addr, &addr_len))
		perror("TcpSocket::TcpSocket getpeername");
}

////////////////////////////////////////////////////////////////////////////////////

TcpSocket::~TcpSocket()
{
}

////////////////////////////////////////////////////////////////////////////////////

bool TcpSocket::Connect()
{
	return Socket::Connect(PF_INET, SOCK_STREAM, (sockaddr*)&addr, sizeof(addr));
}

////////////////////////////////////////////////////////////////////////////////////

const char* TcpSocket::Address()
{
	return inet_ntoa(addr.sin_addr);
}

////////////////////////////////////////////////////////////////////////////////////

int TcpSocket::Port() const
{
	return ntohs(addr.sin_port);
}

////////////////////////////////////////////////////////////////////////////////////

