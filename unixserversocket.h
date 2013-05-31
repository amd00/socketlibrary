
#ifndef _UNIXSERVERSOCKET_H_
#define _UNIXSERVERSOCKET_H_

#include "serversocket.h"
#include "unixsocket.h"

////////////////////////////////////////////////////////////////////////////////////

class UnixServerSocket : public ServerSocket
{
protected:
	virtual void Accept(UnixSocket *sock) = 0;	// Обработка новых подключений
	virtual void Receiver(UnixSocket *sock) = 0;	// Обработка входящих данных

private:
	UnixSocket *GetNewSocket(int fd);			// Создание нового объекта "UnixSocket"
	void Accept(Socket *sock);					// Вызывает Accept(UnixSocket*)
	void Receiver(Socket *sock);					// Вызывает Receiver(UnixSocket*)

public:
	UnixServerSocket(const char *path);			// Конструктор
	virtual ~UnixServerSocket();			// Деструктор
};

////////////////////////////////////////////////////////////////////////////////////

#endif
