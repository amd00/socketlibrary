
#ifndef _TCPSERVERSOCKET_H_
#define _TCPSERVERSOCKET_H_

#include "serversocket.h"
#include "tcpsocket.h"

class TcpServerSocket : public ServerSocket
{
public:
	TcpServerSocket(int _port);
	virtual ~TcpServerSocket() {}
	
protected:
	virtual void accept(TcpSocket *_sock) = 0;
	virtual void receiver(TcpSocket *_sock) = 0;

private:
	TcpSocket *getNewSocket(int _fd) const { return new TcpSocket(_fd); }
	void accept(Socket *_sock) { accept((TcpSocket*)_sock); }
	void receiver(Socket *_sock) { receiver((TcpSocket*)_sock); }
};

#endif
