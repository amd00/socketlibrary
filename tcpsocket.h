
#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

 #include <netinet/in.h>
 #include <arpa/inet.h>

#include "socket.h"

class TcpSocket : public Socket
{
	friend class TcpServerSocket;

private:
	sockaddr_in m_addr;

public:
	TcpSocket(const char *_ip, int _port);
	~TcpSocket() {}

	bool connect() { return Socket::connect(PF_INET, SOCK_STREAM, (sockaddr*)&m_addr, sizeof(m_addr)); }
	const char *address() const { return inet_ntoa(m_addr.sin_addr); }
	int port() const { return ntohs(m_addr.sin_port); }
	
private:
	TcpSocket(int _fd);
};

#endif
