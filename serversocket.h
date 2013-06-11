
/*
 *  Copyright (C) 2013 Andrey Dudakov
 *
 *  Authors: Andrey "amd00" Dudakov
 *
 *  This file is part of socketlibrary.
 *
 *  socketlibrary is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  socketlibrary is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with socketlibrary.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_

#include <signal.h>
#include <map>

#include "socket.h"

class ServerSocket;

typedef std::map<int, Socket*> SocketMap;
typedef std::map<int, ServerSocket*> ServerSocketMap;

#define ACCEPTSIG SIGRTMIN + 5
#define RECVSIG SIGRTMIN + 6

class ServerSocket
{
private:
	static pthread_mutex_t m_mutex;
	static ServerSocketMap m_servers;
	
	SocketMap m_sockets;
	pollfd m_fds[SOMAXCONN + 1];

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
	virtual Socket *getNewSocket(int _fd, sockaddr *_addr) const = 0;
	virtual void disconnectClient(Socket *_sock);
	
private:
	static void asyncAccept(int _s, siginfo_t *_si, void*);
	static void asyncReceiver(int _s, siginfo_t *_si, void*);
	static void *threadAccept(void *_data);
	static void *threadReceiver(void *_data);
	static ServerSocket *findServer(int _fd);
	
	void startListen();
};

// Для реализации асинхронного ввода-вывода используются сигналы SIGRTMIN + 5 и SIGRTMIN + 6

#endif
