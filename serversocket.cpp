
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "serversocket.h"

////////////////////////////////////////////////////////////////////////////////////

ServerSocket::SocketList::SocketList() : first(NULL), size(0)
{
}

////////////////////////////////////////////////////////////////////////////////////

ServerSocket::SocketList::~SocketList()
{
	SocketListItem *tmp_item = first;
	while(tmp_item)
	{
		delete tmp_item -> sock;
		tmp_item = tmp_item -> next;
		delete tmp_item;
	}
}

////////////////////////////////////////////////////////////////////////////////////

size_t ServerSocket::SocketList::Size() const
{
	return size;
}

////////////////////////////////////////////////////////////////////////////////////

bool ServerSocket::SocketList::AddItem(Socket *s)
{
	if(!s)
	{
		fprintf(stderr, "SocketList::AddItem: В качестве аргумента передан NULL\n");
		return false;
	}
	SocketListItem *new_item = new SocketListItem;
	new_item -> sock = s;
	new_item -> next = NULL;
	if(!first)
	{
		first = new_item;
		size++;
		return true;
	}
	SocketListItem *tmp_item = first;
	while(tmp_item -> next)
		tmp_item = tmp_item -> next;
	tmp_item -> next = new_item;
	size++;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////

bool ServerSocket::SocketList::RemoveItem(Socket *s)
{
	if(!s || !size)
		return false;
	SocketListItem *tmp_item = first;
	SocketListItem *tmp_tmp_item = NULL;
	while(tmp_item)
	{
		if(tmp_item -> sock == s)
			break;
		tmp_tmp_item = tmp_item;
		tmp_item = tmp_item -> next;
	}
	if(!tmp_item)
		return false;
	if(!tmp_tmp_item)
		first = tmp_item -> next;
	else
		tmp_tmp_item -> next = tmp_item -> next;
	delete tmp_item;
	size--;
	if(!size)
		first = NULL;
	s = NULL;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////

bool ServerSocket::SocketList::Exists(Socket *s)
{
	if(!s)
		return false;
	SocketListItem *tmp_item = first;
	while(tmp_item)
	{
		if(tmp_item -> sock == s)
			return true;
		tmp_item = tmp_item -> next;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////

Socket *ServerSocket::SocketList::operator[](int i) const
{
	if(i < 0 || i >= (int)size)
		return NULL;
	SocketListItem *tmp_item = first;
	
	for(int j = 0; j < i; j++, tmp_item = tmp_item -> next)
	{
	}
	return tmp_item -> sock;
}

Socket *ServerSocket::SocketList::GetSocketByFd(int fd) const
{
	if(fd <= 0)
		return NULL;
	for(int i = 0; i < (int)size; i++)
		if(operator[](i) -> Fd() == fd)
			return operator[](i);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

 pthread_mutex_t ServerSocket::mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

////////////////////////////////////////////////////////////////////////////////////

ServerSocket::ServerSocketList *ServerSocket::servers = new ServerSocket::ServerSocketList();

ServerSocket::ServerSocket()
{
// Конструктор
	sockets = new ServerSocket::SocketList();
	servers -> AddItem(this);
}

////////////////////////////////////////////////////////////////////////////////////

ServerSocket::~ServerSocket()
{
// Деструктор
	servers -> RemoveItem(this);
	delete sockets;
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
	sockets -> AddItem(GetNewSocket(listener));
// Основной цикл
	while(true)
	{
		memset(polls, 0, sizeof(polls));
// Инициализация массива структур "pollfd"
 		for(unsigned i = 0; i < sockets -> Size(); i++)
		{
			pollfd pol;
			pol.fd = sockets -> operator[](i) -> Fd();
			pol.events = POLLIN;
			polls[i] = pol;
		}
// Ожидание события на файловом дескрипторе

		int res;
		res = TEMP_FAILURE_RETRY(poll(polls, sockets -> Size(), -1));
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
			sockets -> AddItem(s);
		// Метод, определённый пользователем
			Accept(s);
			continue;
		}
// Обработка события на сокете
		int processed_events = 0;
	// Поиск сокета, на котором произощло событие
		for(int i = 0; i < (int)sockets -> Size() && processed_events < res; i++)
			if(polls[i].revents != 0)
			{
				Socket *tmp_socket = sockets -> GetSocketByFd(polls[i].fd);
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
	ServerSocket *self = servers -> GetServerByFd(l);
	self -> sockets -> AddItem(self -> GetNewSocket(l));
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
		self -> sockets -> AddItem(s);
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
	ServerSocket *self = servers -> GetServerBySocketFd(sock -> Fd());
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
	if(sockets -> RemoveItem(socket))
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
		
	sockets -> AddItem(GetNewSocket(listener));
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::AsyncAccept(int sig, siginfo_t *sig_info, void*)
{
// Обработка события на серверном сокете
	ServerSocket *self = servers -> GetServerByFd(sig_info -> si_fd);
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
	self -> sockets -> AddItem(s);
// Метод, определённоый пользователем
	self -> Accept(s);
}

////////////////////////////////////////////////////////////////////////////////////

void ServerSocket::AsyncReceiver(int sig, siginfo_t *sig_info, void*)
{
	ServerSocket *self = servers -> GetServerBySocketFd(sig_info -> si_fd);
	if(!self)
		return;
	if(sig_info -> si_code == POLL_ERR)
	{
		fprintf(stderr, "%s\n", "SIGPOLL вернул POLL_ERR");
		return;
	}
	Socket *tmp_socket = self -> sockets -> GetSocketByFd(sig_info -> si_fd);
	if(!tmp_socket)
		return;
	self -> Receiver(tmp_socket);
	return;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ServerSocket::ServerSocketList::ServerSocketList() : first(NULL), size(0)
{
}

ServerSocket::ServerSocketList::~ServerSocketList()
{
}

bool ServerSocket::ServerSocketList::AddItem(ServerSocket *s)
{
	if(!s)
	{
		fprintf(stderr, "SocketList::AddItem: В качестве аргумента передан NULL\n");
		return false;
	}
	if(!first)
	{
		first = new ServerSocketListItem;
		first -> sock = s;
		first -> next = NULL;
		size++;
		return true;
	}
	ServerSocketListItem *tmp_item = first;
	while(tmp_item -> next)
		tmp_item = tmp_item -> next;
	ServerSocketListItem *new_item;
	new_item = new ServerSocketListItem;
	new_item -> sock = s;
	new_item -> next = NULL;
	tmp_item -> next = new_item;
	size++;
	return true;
}

void ServerSocket::ServerSocketList::RemoveItem(ServerSocket *s)
{
	if(!s || !size)
		return;
	ServerSocketListItem *tmp_item = first;
	ServerSocketListItem *tmp_tmp_item = NULL;
	while(tmp_item)
	{
		if(tmp_item -> sock == s)
			break;
		tmp_tmp_item = tmp_item;
		tmp_item = tmp_item -> next;
	}
	if(!tmp_item)
		return;
	if(!tmp_tmp_item)
		first = tmp_item -> next;
	else
		tmp_tmp_item -> next = tmp_item -> next;
	delete tmp_item;
	size--;
	if(!size)
		first = NULL;
}

size_t ServerSocket::ServerSocketList::Size() const
{
	return size;
}

ServerSocket *ServerSocket::ServerSocketList::operator[](int i) const
{
	if(i < 0 || i >= (int)size)
		return NULL;
	ServerSocketListItem *tmp_item = first;
	
	for(int j = 0; j < i; j++, tmp_item = tmp_item -> next)
	{
	}
	return tmp_item -> sock;
}

ServerSocket *ServerSocket::ServerSocketList::GetServerByFd(int fd) const
{
	if(fd <= 0)
		return NULL;
	
	for(int i = 0; i < (int)size; i++)
	{
		if(this -> operator[](i) -> listener == fd)
			return this -> operator[](i);
	}
	return NULL;
}

ServerSocket *ServerSocket::ServerSocketList::GetServerBySocketFd(int fd) const
{
	if(fd <= 0)
		return NULL;
	
	for(int i = 0; i < (int)size; i++)
	{
		for(int j = 0; j < (int)this -> operator[](i) -> sockets -> Size(); j++)
			if(this -> operator[](i) -> sockets -> operator[](j) -> Fd() == fd)
				return this -> operator[](i);
	}
	return NULL;
}