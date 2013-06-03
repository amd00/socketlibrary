
#include <string.h>
#include <stdio.h>

#include "tcpsocket.h"

////////////////////////////////////////////////////////////////////////////////////

TcpSocket::TcpSocket(const char *_ip_addr, int _port) : Socket()
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(_port);
	if(!inet_aton(_ip_addr ? _ip_addr : "127.0.0.1", &m_addr.sin_addr))
		fprintf(stderr, "Incorrect address \"%s\"\n", _ip_addr);
}

TcpSocket::TcpSocket(int _fd) : Socket(_fd)
{
	memset(&m_addr, 0, sizeof(m_addr));
	socklen_t addr_len = sizeof(m_addr);
	if(getpeername(_fd, (sockaddr*)&m_addr, &addr_len))
		perror("TcpSocket::TcpSocket getpeername");
}
