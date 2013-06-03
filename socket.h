
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <sys/poll.h>

class Socket
{
	friend class ServerSocket;

private:
	pollfd m_fds;
	int m_last_error;
	bool m_connected;

protected:
	bool connect(int _domain, int _type, sockaddr *_addr, socklen_t _sl);
	Socket(int _fd);

public:
	Socket();
	virtual ~Socket();
	virtual bool connect() = 0;
	int send(const void *data, size_t data_size, int timeout = -1);
	int recv(void *data, size_t data_size, int timeout = -1);
	void waitData(int timeout = -1);
	void disconnect();
	virtual const char *address() const = 0;
	bool connected() const { return m_connected; }
	int fd() const { return m_fds.fd; }
	
private:
	int syncSend(const void *data, size_t data_size, int timeout = -1);
	int asyncSend(const void *data, size_t data_size, int timeout = -1);
	int syncRecv(void *data, size_t data_size, int timeout = -1);
	int asyncRecv(void *data, size_t data_size, int timeout = -1);
};

#endif

