
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

class SocketComparer
{
public:
	bool operator()(Socket *_sock, int _fd)
	{
		return (_sock -> Fd() == _fd);
	};
} sock_comparer;

pthread_mutex_t ServerSocket::mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

ServerSocket *ServerSocket::findServer(int _fd)
{
	for(ServerSocketMap::iterator it = servers.begin(); it != servers.end(); it++)
	{
		if(it -> second -> sockets.count(_fd))
			return it -> second;
	}
	return NULL;
}

ServerSocket::ServerSocket() : sockets()
{
// Конструктор
// 	servers.push_back(this);
}

////////////////////////////////////////////////////////////////////////////////////

ServerSocket::~ServerSocket()
{
// Деструктор
// 	servers.RemoveItem(this);
	servers.erase(listener);
	std::for_each(sockets.begin(), sockets.end(), sock_eraser);
	close(listener);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::Start()
{
	srv_type = Sync;
// Запуск синхронной обработки событий
// Инициализация неблокируемого серверного сокета
	if(fcntl(listener, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("ServerSocket::Start fcntl");
		return;
	}
	const int on = 1;
// Принудительное освобождение порта при запуске
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		perror("ServerSocket::Start setsockopt");
		return;
	}
// Привязка сокета к порту
	if(bind(listener, &addr, sizeof(addr)) == -1)
	{
		perror("ServerSocket::Start bind");
		return;
	}
// Запуск прослушивания
	if(listen(listener, SOMAXCONN) == -1)
	{
		perror("ServerSocket::Start listen");
		return;
	}
	Socket *new_sock = GetNewSocket(listener);
	servers[listener] = this;
	sockets[new_sock -> Fd()] = new_sock;
// Основной цикл
	while(true)
	{
		memset(polls, 0, sizeof(polls));
// Инициализация массива структур "pollfd"
		int i = 0;
 		for(SocketMap::iterator it = sockets.begin(); it != sockets.end(); it++, i++)
		{
			pollfd pol;
			pol.fd = (it -> second) -> Fd();
			pol.events = POLLIN;
			polls[i] = pol;
		}
// Ожидание события на файловом дескрипторе

		int res;
		res = TEMP_FAILURE_RETRY(poll(polls, sockets.size(), -1));
		if(res == -1)
		{
			perror("ServerSocket::Start poll");
			continue;
		}
		if( polls[0].revents != 0)
		{
		// Обработка события на серверном сокете
			sockaddr a;
			memset(&a, 0, sizeof(a));
			socklen_t la = sizeof(a);
		// Инициализация подключённого неблокируемого сокета
			int sock = -1;
			sock = TEMP_FAILURE_RETRY(accept(listener, &a, &la));
			if(sock == -1)
			{
				perror("ServerSocket::Start accept");
				continue;
			}
			Socket *s = GetNewSocket(sock);
			if(!s)
			{
				close(sock);
				continue;
			}
			sockets[s -> Fd()] = s;
		// Метод, определённый пользователем
			Accept(s);
			continue;
		}
// Обработка события на сокете
		int processed_events = 0;
	// Поиск сокета, на котором произощло событие
		for(int i = 0; i < (int)sockets.size() && processed_events < res; i++)
			if(polls[i].revents != 0)
			{
				Socket *tmp_socket = sockets[polls[i].fd];
				Receiver(tmp_socket);
				processed_events++;
			}
	}
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::ThreadStart()
{
	srv_type = Thread;
	// Запуск синхронной обработки событий
// Инициализация неблокируемого серверного сокета
	if(fcntl(listener, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("ServerSocket::Start fcntl");
		return;
	}
	const int on = 1;
// Принудительное освобождение порта при запуске
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		perror("ServerSocket::Start setsockopt");
		return;
	}
// Привязка сокета к порту
	if(bind(listener, &addr, sizeof(addr)) == -1)
	{
		perror("ServerSocket::Start bind");
		return;
	}
// Запуск прослушивания
	if(listen(listener, SOMAXCONN) == -1)
	{
		perror("ServerSocket::Start listen");
		return;
	}
// Создание потока, слушающего порт
	pthread_t p_th;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&p_th, &attr, ServerSocket::ThreadAccept, &listener);
}

////////////////////////////////////////////////////////////////////////////////////

void *ServerSocket::ThreadAccept(void *data)
{
	int l = *(int*)data;
	ServerSocket *self = servers[l];
	self -> sockets[l] = self -> GetNewSocket(l);
	pollfd listener_polls[1];
	listener_polls[0].fd = l;
	listener_polls[0].events = POLLIN;
	while(true)
	{
		int res;
		res = TEMP_FAILURE_RETRY(poll(listener_polls, 1, -1));
		if(res == -1)
		{
			perror("ServerSocket::ThreadAccept poll");
			continue;
		}
	// Обработка события на серверном сокете
		sockaddr a;
		memset(&a, 0, sizeof(a));
		socklen_t la = sizeof(a);
	// Инициализация подключённого сокета
		
		int sock = TEMP_FAILURE_RETRY(accept(l, &a, &la));
		if(sock == -1)
		{
			perror("ServerSocket::ThreadAccept accept");
			continue;
		}
		pthread_mutex_lock(&mutex);
		Socket *s = self -> GetNewSocket(sock);
		if(!s)
		{
			close(sock);
			pthread_mutex_unlock(&mutex);
			continue;
		}
		self -> sockets[sock] = s;
	// Метод, определённый пользователем
		self -> Accept(s);
		pthread_mutex_unlock(&mutex);
		
	// Создание потока, принимающего данные
		pthread_t p_th;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&p_th, &attr, ServerSocket::ThreadReceiver, s);
		continue;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////

void *ServerSocket::ThreadReceiver(void *data)
{
	Socket *sock = (Socket*)data;
	pthread_mutex_lock(&mutex);
// #error 
	ServerSocket *self = findServer(sock -> Fd());
 	pthread_mutex_unlock(&mutex);
	if(!self)
		pthread_exit(NULL);
	while(true)
	{
		sock -> WaitData();
		self -> Receiver(sock);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::DisconnectClient(Socket *socket)
{
// Отключение клиента
	pthread_mutex_lock(&mutex);
	if(sockets.erase(socket -> Fd()))
	{
		delete socket;
		socket = NULL;
		pthread_mutex_unlock(&mutex);
		if(srv_type == Thread)
		{
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		}
	}
   	pthread_mutex_unlock(&mutex);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::AsyncStart()
{
	srv_type = Async;
// Запуск асинхронной обработки событий
// Инициализация асинхронного серверного сокета
	if(fcntl(listener, F_SETFL, O_ASYNC) == -1 || 
		fcntl(listener, F_SETOWN, getpid()) == -1 || 
		fcntl(listener, F_SETSIG, SIGRTMIN + 5) == -1)
	{
		perror("ServerSocket::AsyncStart fcntl");
		return;
	}
// Определение обработчика сигнала ввода-вывода
	
	struct sigaction sa_accept, sa_recv;
	
	sa_accept.sa_sigaction = ServerSocket::AsyncAccept;
	sa_accept.sa_flags = SA_RESTART | SA_SIGINFO;
	
	sa_recv.sa_sigaction = ServerSocket::AsyncReceiver;
	sa_recv.sa_flags = SA_RESTART | SA_SIGINFO;
	
	if(sigaddset(&sa_accept.sa_mask, SIGRTMIN + 5) == -1 || 
		  sigaddset(&sa_accept.sa_mask, SIGRTMIN + 6) == -1 ||
		  sigaddset(&sa_recv.sa_mask, SIGRTMIN + 5) == -1 || 
		  sigaddset(&sa_recv.sa_mask, SIGRTMIN + 6) == -1)
	{
		perror("ServerSocket::AsyncStart sigaddset");
		return;
	}
	if(sigaction(SIGRTMIN + 5, &sa_accept, NULL) == -1 || 
		  sigaction(SIGRTMIN + 6, &sa_recv, NULL) == -1)
	{
		perror("ServerSocket::AsyncStart sigaction");
		return;
	}
	const int on = 1;
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		perror("ServerSocket::AsyncStart setsockopt");
		return;
	}	
	if(bind(listener, &addr, sizeof(addr)) == -1)
	{
		perror("ServerSocket::AsyncStart bind");
		return;
	}
		
	if(listen(listener, SOMAXCONN) == -1)
	{
		perror("ServerSocket::AsyncStart listen");
		return;
	}
	
	sockets[listener] =GetNewSocket(listener);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::AsyncAccept(int sig, siginfo_t *sig_info, void*)
{
// Обработка события на серверном сокете
	ServerSocket *self = servers[sig_info -> si_fd];
	if(!self)
		return;
	if(sig_info -> si_code == POLL_ERR)
	{
		fprintf(stderr, "%s\n", "SIGPOLL вернул POLL_ERR");
		return;
	}
	sockaddr a;
	memset(&a, 0, sizeof(a));
	socklen_t la = sizeof(a);
// Инициализация подключённого асинхронного сокета
	int sock = TEMP_FAILURE_RETRY(accept(sig_info -> si_fd, &a, &la));
	if(sock == -1)
	{
		perror("ServerSocket::AsyncReceiver accept");
		return;
	}
	if(fcntl(sock, F_SETFL, FASYNC) == -1 ||
		  fcntl(sock, F_SETOWN, getpid()) == -1 ||
		  fcntl(sock, F_SETSIG, SIGRTMIN + 6) == -1)
	{
		perror("ServerSocket::AsyncReceiver fcntl");
		close(sock);
		return;
	}
	Socket *s = self -> GetNewSocket(sock);
	if(!s)
	{
		close(sock);
		return;
	}
	self -> sockets[sock] = s;
// Метод, определённоый пользователем
	self -> Accept(s);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::AsyncReceiver(int sig, siginfo_t *sig_info, void*)
{
	ServerSocket *self = findServer(sig_info -> si_fd);
	if(!self)
		return;
	if(sig_info -> si_code == POLL_ERR)
	{
		fprintf(stderr, "%s\n", "SIGPOLL вернул POLL_ERR");
		return;
	}
	Socket *tmp_socket = self -> sockets[sig_info -> si_fd];
	if(!tmp_socket)
		return;
	self -> Receiver(tmp_socket);
	return;
}
