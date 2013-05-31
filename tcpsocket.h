
#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

 #include <netinet/in.h>

#include "socket.h"

////////////////////////////////////////////////////////////////////////////////////

class TcpSocket : public Socket
{
	friend class TcpServerSocket;

private:
	sockaddr_in addr;
	TcpSocket(int fd);

public:
	TcpSocket(const char *ip, int port);
	~TcpSocket();
	bool Connect();
	const char *Address();
	int Port() const;
};

////////////////////////////////////////////////////////////////////////////////////

#endif
