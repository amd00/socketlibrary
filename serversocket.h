
#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_

#include <signal.h>
#include <map>

#include "socket.h"

class ServerSocket;

typedef std::map<int, Socket*> SocketMap;
typedef std::map<int, ServerSocket*> ServerSocketMap;

class ServerSocket
{
private:
	static pthread_mutex_t m_mutex;
	static ServerSocketMap m_servers;
	
	SocketMap m_sockets;
	pollfd m_fds[SOMAXCONN+1];

protected:
	enum Type
	{
		Sync,
		Async,
		Thread
	};
	int m_listener;
	sockaddr m_addr;
	Type m_type;

public:
	ServerSocket();
	virtual ~ServerSocket();
	
	void start();
	void asyncStart();
	void threadStart();

protected:
	virtual void accept(Socket *_sock) = 0;
	virtual void receiver(Socket *_sock) = 0;
	virtual Socket *getNewSocket(int _fd) const = 0;
	virtual void disconnectClient(Socket *_sock);
	
private:
	static void asyncAccept(int _s, siginfo_t *_si, void*);
	static void asyncReceiver(int _s, siginfo_t *_si, void*);
	static void *threadAccept(void *_data);
	static void *threadReceiver(void *_data);
	static ServerSocket *findServer(int _fd);
};

// Для реализации асинхронного ввода-вывода используются сигналы SIGRTMIN + 5 и SIGRTMIN + 6

#endif
