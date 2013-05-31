
#ifndef _TCPSERVERSOCKET_H_
#define _TCPSERVERSOCKET_H_

#include "serversocket.h"
#include "tcpsocket.h"

////////////////////////////////////////////////////////////////////////////////////

class TcpServerSocket : public ServerSocket
{
protected:
	virtual void Accept(TcpSocket *sock) = 0;	// Обработка новых подключений
	virtual void Receiver(TcpSocket *sock) = 0;	// Обработка входящих данных

private:
	TcpSocket *GetNewSocket(int fd);			// Создание нового объекта "TcpSocket"
	void Accept(Socket *sock);					// Вызывает Accept(TcpSocket*)
	void Receiver(Socket *sock);					// Вызывает Receiver(TcpSocket*)

public:
	TcpServerSocket(int port);					// Конструктор
	virtual ~TcpServerSocket();				// Деструктор
};

////////////////////////////////////////////////////////////////////////////////////

#endif
