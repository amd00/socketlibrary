
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <sys/poll.h>

////////////////////////////////////////////////////////////////////////////////////

class Socket
{
	friend class ServerSocket;

private:
	int sock;										// Файловый дескриптор
	pollfd fds;									// Массив структур "pollfd"
	int last_error;
	bool connected;									// Состояние сокета
	int SyncSend(const void *data, size_t data_size, int timeout = -1);
	int AsyncSend(const void *data, size_t data_size, int timeout = -1);
	int SyncRecv(void *data, size_t data_size, int timeout = -1);
	int AsyncRecv(void *data, size_t data_size, int timeout = -1);
protected:
	bool Connect(int domain, int type, sockaddr *addr, socklen_t sl);	// Метод, осуществляющий соединение
	Socket(int fd);									// Конструктор
public:
	Socket();										// Конструктор
	virtual ~Socket();								// Деструктор
	virtual bool Connect() = 0;
	int Send(const void *data, size_t data_size, int timeout = -1);// Метод, осуществляющий посылку данных
	int Recv(void *data, size_t data_size, int timeout = -1);		// Метод, осуществляющий приём данных
	void WaitData(int timeout = -1);
	void Disconnect();								// Разрыв соединения
	virtual const char *Address() = 0;				// Удалённый адрес (путь к файлу)
	bool Connected() const;							// Метод, возвращающий состояние сокета
	int Fd() const;									// Метод, возвращающий файловый дескриптор
};

////////////////////////////////////////////////////////////////////////////////////

#endif

