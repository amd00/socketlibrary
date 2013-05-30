
#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_

#include <signal.h>
#include <map>

#include "socket.h"

class ServerSocket;

typedef std::map<int, Socket*> SocketMap;
typedef std::map<int, ServerSocket*> ServerSocketMap;

////////////////////////////////////////////////////////////////////////////////////

class ServerSocket
{
private:
	static pthread_mutex_t mutex;
	SocketMap sockets;
	pollfd polls[SOMAXCONN+1];
	static void AsyncAccept(int s, siginfo_t *si, void*);
	static void AsyncReceiver(int s, siginfo_t *si, void*);
	static void *ThreadAccept(void *data);
	static void *ThreadReceiver(void *data);
	static ServerSocketMap servers;
	static ServerSocket *findServer(int _fd);

protected:
	enum Type
	{
		Sync, /*!< синхронный*/ 
		Async, /*!< асинхронный*/
		Thread /*!< многопоточный*/
	};
	int listener;
	sockaddr addr;
	Type srv_type;
	virtual void Accept(Socket *sock) = 0;
	virtual void Receiver(Socket *sock) = 0;
	virtual Socket *GetNewSocket(int fd) = 0;
	virtual void DisconnectClient(Socket *sock);

public:
	ServerSocket();
	virtual ~ServerSocket();
	void Start();
	void AsyncStart();
	void ThreadStart();
};



// Для реализации асинхронного ввода-вывода используются сигналы SIGRTMIN + 5 и SIGRTMIN + 6

////////////////////////////////////////////////////////////////////////////////////

#endif
