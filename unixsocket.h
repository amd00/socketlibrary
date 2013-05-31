
#ifndef _UNIXSOCKET_H_
#define _UNIXSOCKET_H_

#include <sys/un.h>

#include "socket.h"

////////////////////////////////////////////////////////////////////////////////////

class UnixSocket : public Socket
{

	friend class UnixServerSocket;

private:
	sockaddr_un addr;
	UnixSocket(int fd);

public:
	UnixSocket(const char *path);
	~UnixSocket();
	bool Connect();
	const char *Address();
};

////////////////////////////////////////////////////////////////////////////////////

#endif
