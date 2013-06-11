
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

#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

#include "serversocket.h"

class SocketEraser
{
public:
	void operator() (std::pair<const int, Socket*> &_val)
	{
		delete _val.second;
	};
} sock_eraser;

/************************************************************************/

pthread_mutex_t ServerSocket::m_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

ServerSocketMap ServerSocket::m_servers;

ServerSocket *ServerSocket::findServer(int _fd)
{
	for(ServerSocketMap::iterator it = m_servers.begin(); it != m_servers.end(); it++)
	{
		if(it -> second -> m_sockets.count(_fd))
			return it -> second;
	}
	return NULL;
}

ServerSocket::ServerSocket() : m_sockets()
{
}

////////////////////////////////////////////////////////////////////////////////////

ServerSocket::~ServerSocket()
{
// Деструктор
// 	servers.RemoveItem(this);
	m_servers.erase(m_listener);
	std::for_each(m_sockets.begin(), m_sockets.end(), sock_eraser);
	::close(m_listener);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::startListen()
{
	const int on = 1;
// Принудительное освобождение порта при запуске
	if(::setsockopt(m_listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		perror("ServerSocket::Start setsockopt");
		return;
	}
// Привязка сокета к порту
	if(::bind(m_listener, &m_addr, sizeof(m_addr)) == -1)
	{
		perror("ServerSocket::Start bind");
		return;
	}
// Запуск прослушивания
	if(::listen(m_listener, SOMAXCONN) == -1)
	{
		perror("ServerSocket::Start listen");
		return;
	}
// 	Socket *new_sock = getNewSocket(m_listener);
	m_servers[m_listener] = this;
// 	m_sockets[new_sock -> fd()] = new_sock;
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::start()
{
	m_type = Sync;
// Запуск синхронной обработки событий
// Инициализация неблокируемого серверного сокета
	::fcntl(m_listener, F_SETFL, O_NONBLOCK);
	
	Socket *new_sock = getNewSocket(m_listener, NULL);
	m_sockets[new_sock -> fd()] = new_sock;
// Start listen
	startListen();
// Основной цикл
	while(true)
	{
		::memset(m_fds, 0, sizeof(m_fds));
// Инициализация массива структур "pollfd"
		int i = 0;
 		for(SocketMap::iterator it = m_sockets.begin(); it != m_sockets.end(); it++, i++)
		{
			pollfd pol;
			pol.fd = it -> first;
			pol.events = POLLIN;
			m_fds[i] = pol;
		}
// Ожидание события на файловом дескрипторе

		int res = ::poll(m_fds, m_sockets.size(), -1);
		if(res == -1)
		{
			perror("ServerSocket::Start poll");
			continue;
		}
		if(m_fds[0].revents != 0)
		{
		// Обработка события на серверном сокете
			sockaddr *a = new sockaddr;
			memset(a, 0, sizeof(sockaddr));
			socklen_t la = sizeof(sockaddr);
		// Инициализация подключённого неблокируемого сокета
			int sock = ::accept(m_listener, a, &la);
			if(sock == -1)
			{
				perror("ServerSocket::Start accept");
				delete a;
				continue;
			}
			::fcntl(sock, F_SETFL, O_NONBLOCK);
			Socket *s = getNewSocket(sock, a);
			if(!s)
			{
				::close(sock);
				delete a;
				continue;
			}
			m_sockets[s -> fd()] = s;
		// Метод, определённый пользователем
			accept(s);
			continue;
		}
// Обработка события на сокете
		int processed_events = 0;
	// Поиск сокета, на котором произощло событие
		for(int i = 0; i < (int)m_sockets.size() && processed_events < res; i++)
			if(m_fds[i].revents != 0)
			{
				Socket *tmp_socket = m_sockets[m_fds[i].fd];
				receiver(tmp_socket);
				processed_events++;
			}
	}
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::threadStart()
{
	m_type = Thread;
	// Запуск синхронной обработки событий
// Инициализация неблокируемого серверного сокета
	if(::fcntl(m_listener, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("ServerSocket::Start fcntl");
		return;
	}
	startListen();
// Создание потока, слушающего порт
	pthread_t p_th;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&p_th, &attr, ServerSocket::threadAccept, &m_listener);
}

////////////////////////////////////////////////////////////////////////////////////

void *ServerSocket::threadAccept(void *_data)
{
	int l = *(int*)_data;
	ServerSocket *self = m_servers[l];
	self -> m_sockets[l] = self -> getNewSocket(l, NULL);
	pollfd listener_polls[1];
	listener_polls[0].fd = l;
	listener_polls[0].events = POLLIN;
	while(true)
	{
		int res = ::poll(listener_polls, 1, -1);
		if(res == -1)
		{
			::perror("ServerSocket::ThreadAccept poll");
			continue;
		}
	// Обработка события на серверном сокете
		sockaddr *a = new sockaddr;
		::memset(a, 0, sizeof(sockaddr));
		socklen_t la = sizeof(sockaddr);
	// Инициализация подключённого сокета
		
		int sock = ::accept(l, a, &la);
		if(sock == -1)
		{
			::perror("ServerSocket::ThreadAccept accept");
			delete a;
			continue;
		}
		pthread_mutex_lock(&m_mutex);
		Socket *s = self -> getNewSocket(sock, a);
		if(!s)
		{
			::close(sock);
			delete a;
			pthread_mutex_unlock(&m_mutex);
			continue;
		}
		self -> m_sockets[sock] = s;
	// Метод, определённый пользователем
		self -> accept(s);
		pthread_mutex_unlock(&m_mutex);
		
	// Создание потока, принимающего данные
		pthread_t p_th;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&p_th, &attr, ServerSocket::threadReceiver, s);
		continue;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////

void *ServerSocket::threadReceiver(void *_data)
{
	Socket *sock = (Socket*)_data;
	pthread_mutex_lock(&m_mutex);
// #error 
	ServerSocket *self = findServer(sock -> fd());
 	pthread_mutex_unlock(&m_mutex);
	if(!self)
		pthread_exit(NULL);
	while(true)
	{
		sock -> waitData();
		self -> receiver(sock);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::disconnectClient(Socket *_socket)
{
// Отключение клиента
	pthread_mutex_lock(&m_mutex);
	if(m_sockets.erase(_socket -> fd()))
	{
		::close(_socket -> fd());
		delete _socket;
		_socket = NULL;
		if(m_type == Thread)
		{
			pthread_mutex_unlock(&m_mutex);
			pthread_exit(NULL);
		}
	}
   	pthread_mutex_unlock(&m_mutex);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::asyncStart()
{
	m_type = Async;
// Запуск асинхронной обработки событий
// Инициализация асинхронного серверного сокета
	if(::fcntl(m_listener, F_SETFL, FASYNC) == -1 || 
		::fcntl(m_listener, F_SETOWN, getpid()) == -1 || 
		::fcntl(m_listener, F_SETSIG, ACCEPTSIG) == -1)
	{
		::perror("ServerSocket::AsyncStart fcntl");
		return;
	}
// Определение обработчика сигнала ввода-вывода

	struct sigaction sa_accept, sa_recv;
	
	sa_accept.sa_sigaction = ServerSocket::asyncAccept;
	sa_accept.sa_flags = SA_SIGINFO;
	
	sa_recv.sa_sigaction = ServerSocket::asyncReceiver;
	sa_recv.sa_flags = SA_SIGINFO;
	
	if(::sigaddset(&sa_accept.sa_mask, ACCEPTSIG) == -1 || 
		  ::sigaddset(&sa_accept.sa_mask, SIGIO) == -1 ||
		  ::sigaddset(&sa_recv.sa_mask, ACCEPTSIG) == -1 || 
		  ::sigaddset(&sa_recv.sa_mask, SIGIO) == -1)
	{
		::perror("ServerSocket::AsyncStart sigemptyset");
		return;
	}
	if(::sigaction(ACCEPTSIG, &sa_accept, NULL) == -1 || 
		  ::sigaction(SIGIO, &sa_recv, NULL) == -1)
	{
		::perror("ServerSocket::AsyncStart sigaction");
		return;
	}
	startListen();
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::asyncAccept(int _sig, siginfo_t *_sig_info, void*)
{
// Обработка события на серверном сокете
	ServerSocket *self = m_servers[_sig_info -> si_fd];
	if(!self)
		return;
	if(_sig_info -> si_code == POLL_ERR)
	{
		fprintf(stderr, "%s\n", "SIGPOLL вернул POLL_ERR");
		return;
	}
	sockaddr *a = new sockaddr;
	::memset(a, 0, sizeof(sockaddr));
	socklen_t la = sizeof(sockaddr);
// Инициализация подключённого асинхронного сокета
	int sock = ::accept(_sig_info -> si_fd, a, &la);
	if(sock == -1)
	{
		::perror("ServerSocket::AsyncReceiver accept");
		delete a;
		return;
	}
	if(::fcntl(sock, F_SETFL, FASYNC) == -1 ||
		  ::fcntl(sock, F_SETOWN, getpid()) == -1 ||
		  ::fcntl(sock, F_SETSIG, SIGIO) == -1)
	{
		::perror("ServerSocket::AsyncReceiver fcntl");
		::close(sock);
		delete a;
		return;
	}
	Socket *s = self -> getNewSocket(sock, a);
	if(!s)
	{
		::close(sock);
		delete a;
		return;
	}
	self -> m_sockets[sock] = s;
// Метод, определённоый пользователем
	self -> accept(s);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::asyncReceiver(int _sig, siginfo_t *_sig_info, void*)
{
	fprintf(stderr, "111\n");
	ServerSocket *self = findServer(_sig_info -> si_fd);
	if(!self)
		return;
	if(_sig_info -> si_code == POLL_ERR)
	{
		fprintf(stderr, "%s\n", "SIGPOLL вернул POLL_ERR");
		return;
	}
	Socket *tmp_socket = self -> m_sockets[_sig_info -> si_fd];
	if(!tmp_socket)
		return;
	self -> receiver(tmp_socket);
}
