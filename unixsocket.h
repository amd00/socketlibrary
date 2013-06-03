
#ifndef _UNIXSOCKET_H_
#define _UNIXSOCKET_H_

#include <sys/un.h>

#include "socket.h"

class UnixSocket : public Socket
{
	friend class UnixServerSocket;

private:
	sockaddr_un m_addr;

public:
	UnixSocket(const char *_path);
	~UnixSocket() {}
	
	bool connect() { return Socket::connect(PF_UNIX, SOCK_STREAM, (sockaddr*)&m_addr, sizeof(m_addr)); }
	const char *address() const { return m_addr.sun_path; }
	
private:
	UnixSocket(int _fd) : Socket(_fd) {}
};

#endif
