
#ifndef _UNIXSERVERSOCKET_H_
#define _UNIXSERVERSOCKET_H_

#include "serversocket.h"
#include "unixsocket.h"

class UnixServerSocket : public ServerSocket
{
protected:
	virtual void accept(UnixSocket *_sock) = 0;	// Обработка новых подключений
	virtual void receiver(UnixSocket *_sock) = 0;	// Обработка входящих данных

private:
	UnixSocket *getNewSocket(int _fd) { return new UnixSocket(_fd); }
	void accept(Socket *_sock) { accept((UnixSocket*)_sock); }
	void receiver(Socket *_sock) { receiver((UnixSocket*)_sock); }

public:
	UnixServerSocket(const char *_path);			// Конструктор
	virtual ~UnixServerSocket() {}
};

#endif
