
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
// Конструктор
// 	servers.push_back(this);
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
	Socket *new_sock = getNewSocket(m_listener);
	m_servers[m_listener] = this;
	m_sockets[new_sock -> fd()] = new_sock;
}

void ServerSocket::start()
{
	m_type = Sync;
// Запуск синхронной обработки событий
// Инициализация неблокируемого серверного сокета
	if(::fcntl(m_listener, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("ServerSocket::Start fcntl");
		return;
	}
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
			pol.fd = (it -> second) -> fd();
			pol.events = POLLIN;
			m_fds[i] = pol;
		}
// Ожидание события на файловом дескрипторе

		int res;
		res = TEMP_FAILURE_RETRY(::poll(m_fds, m_sockets.size(), -1));
		if(res == -1)
		{
			perror("ServerSocket::Start poll");
			continue;
		}
		if(m_fds[0].revents != 0)
		{
		// Обработка события на серверном сокете
			sockaddr a;
			memset(&a, 0, sizeof(a));
			socklen_t la = sizeof(a);
		// Инициализация подключённого неблокируемого сокета
			int sock = TEMP_FAILURE_RETRY(::accept(m_listener, &a, &la));
			if(sock == -1)
			{
				perror("ServerSocket::Start accept");
				continue;
			}
			Socket *s = getNewSocket(sock);
			if(!s)
			{
				::close(sock);
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
	self -> m_sockets[l] = self -> getNewSocket(l);
	pollfd listener_polls[1];
	listener_polls[0].fd = l;
	listener_polls[0].events = POLLIN;
	while(true)
	{
		int res = TEMP_FAILURE_RETRY(::poll(listener_polls, 1, -1));
		if(res == -1)
		{
			::perror("ServerSocket::ThreadAccept poll");
			continue;
		}
	// Обработка события на серверном сокете
		sockaddr a;
		::memset(&a, 0, sizeof(a));
		socklen_t la = sizeof(a);
	// Инициализация подключённого сокета
		
		int sock = TEMP_FAILURE_RETRY(::accept(l, &a, &la));
		if(sock == -1)
		{
			::perror("ServerSocket::ThreadAccept accept");
			continue;
		}
		pthread_mutex_lock(&m_mutex);
		Socket *s = self -> getNewSocket(sock);
		if(!s)
		{
			::close(sock);
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

void ServerSocket::disconnectClient(Socket *socket)
{
// Отключение клиента
	pthread_mutex_lock(&m_mutex);
	if(m_sockets.erase(socket -> fd()))
	{
		delete socket;
		socket = NULL;
		pthread_mutex_unlock(&m_mutex);
		if(m_type == Thread)
			pthread_exit(NULL);
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
		::fcntl(m_listener, F_SETSIG, SIGRTMIN + 5) == -1)
	{
		::perror("ServerSocket::AsyncStart fcntl");
		return;
	}
// Определение обработчика сигнала ввода-вывода
	
	struct sigaction sa_accept, sa_recv;
	
	sa_accept.sa_sigaction = ServerSocket::asyncAccept;
	sa_accept.sa_flags = SA_RESTART | SA_SIGINFO;
	
	sa_recv.sa_sigaction = ServerSocket::asyncReceiver;
	sa_recv.sa_flags = SA_RESTART | SA_SIGINFO;
	
	if(::sigaddset(&sa_accept.sa_mask, SIGRTMIN + 5) == -1 || 
		  ::sigaddset(&sa_accept.sa_mask, SIGRTMIN + 6) == -1 ||
		  ::sigaddset(&sa_recv.sa_mask, SIGRTMIN + 5) == -1 || 
		  ::sigaddset(&sa_recv.sa_mask, SIGRTMIN + 6) == -1)
	{
		::perror("ServerSocket::AsyncStart sigaddset");
		return;
	}
	if(::sigaction(SIGRTMIN + 5, &sa_accept, NULL) == -1 || 
		  ::sigaction(SIGRTMIN + 6, &sa_recv, NULL) == -1)
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
	sockaddr a;
	::memset(&a, 0, sizeof(a));
	socklen_t la = sizeof(a);
// Инициализация подключённого асинхронного сокета
	int sock = TEMP_FAILURE_RETRY(::accept(_sig_info -> si_fd, &a, &la));
	if(sock == -1)
	{
		::perror("ServerSocket::AsyncReceiver accept");
		return;
	}
	if(::fcntl(sock, F_SETFL, FASYNC) == -1 ||
		  ::fcntl(sock, F_SETOWN, getpid()) == -1 ||
		  ::fcntl(sock, F_SETSIG, SIGRTMIN + 6) == -1)
	{
		::perror("ServerSocket::AsyncReceiver fcntl");
		::close(sock);
		return;
	}
	Socket *s = self -> getNewSocket(sock);
	if(!s)
	{
		::close(sock);
		return;
	}
	self -> m_sockets[sock] = s;
// Метод, определённоый пользователем
	self -> accept(s);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::asyncReceiver(int _sig, siginfo_t *_sig_info, void*)
{
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
	return;
}
