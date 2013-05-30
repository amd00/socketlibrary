
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "socket.h"

////////////////////////////////////////////////////////////////////////////////////

Socket::Socket() : connected(false)
{
}

////////////////////////////////////////////////////////////////////////////////////

Socket::Socket(int fd) : sock(fd), last_error(0), connected(true)
{
// Определение типа сокета (синхронный или асинхронный)
	int sock_flags = fcntl(sock, F_GETFL);
	if(sock_flags == -1)
		perror("Socket::Socket fcntl");
	if(!(sock_flags & O_ASYNC))
	{
		if(fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
			perror("Socket::Socket fcntl");
	}
	fds.fd = sock;
}

////////////////////////////////////////////////////////////////////////////////////

Socket::~Socket()
{
	Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////

bool Socket::Connect(int domain, int type, sockaddr *addr, socklen_t sock_len)
{
	if(connected)
		return false;
	sock = socket(domain, type, 0);
	if(sock == -1)
	{
		perror("Socket::Connect socket");
		return false;
	}
	fds.fd = sock;
	int conn_res = 0;
// Подключение к серверу
	if((conn_res = TEMP_FAILURE_RETRY(connect(sock, addr, sock_len))) == -1)
	{
		perror("Socket::Connect connect");
		if(errno == EISCONN)
		{
			connected = true;
			return false;
		}
		Disconnect();
		return false;
	}
// Создание неблокируемого сокета
	if(fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("Socket::Connect fcntl");
 		Disconnect();
		return false;
	}
	connected = true;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////

void Socket::Disconnect()
{
// Закрытие сокета
	connected = false;
	if(close(sock) == -1)
		perror("Socket::Disconnect close");
	sock = -1;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::SyncSend(const void* data, size_t size, int timeout)
{
// Синхронная посылка данных
	if(!data)
		return -1;
	char *char_data = (char*)data;
	fds.events = POLLOUT;
	int sent_bytes = 0;
	int poll_res;
	while(sent_bytes < (int)size)
	{
		int bytes_for_send = size - sent_bytes;
		poll_res = TEMP_FAILURE_RETRY(poll(&fds, 1, timeout));
		if(poll_res <= 0)
		{
			if(poll_res == 0)
				errno = ETIMEDOUT;
			perror("Socket::SyncSend poll");
			return -1;
		}
		if(fds.revents & POLLHUP || fds.revents & POLLERR)
		{
 			Disconnect();
			return 0;
		}
		int tmp_bytes;
		tmp_bytes = TEMP_FAILURE_RETRY(send(sock, &char_data[sent_bytes], bytes_for_send, MSG_NOSIGNAL));
		if(tmp_bytes == -1)
		{
			perror("Socket::SyncSend send");
			return -1;
		}
		sent_bytes += tmp_bytes;
	}
	return sent_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::AsyncSend(const void* data, size_t size, int timeout)
{
	int tmp_bytes;
	tmp_bytes = TEMP_FAILURE_RETRY(send(sock, data, size, MSG_NOSIGNAL));
	if(tmp_bytes == -1)
	{
		perror("Socket::AsyncSend send");
		if(errno == EPIPE)
		{
			Disconnect();
			tmp_bytes = 0;
		}
	}
	return tmp_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::Send(const void* data, size_t size, int timeout)
{
	if(!data)
		return -1;
	if(fcntl(sock, F_GETFL) & O_NONBLOCK)
		return SyncSend(data, size, timeout);
	if(fcntl(sock, F_GETFL) & O_ASYNC)
		return AsyncSend(data, size, timeout);
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::SyncRecv(void *data, size_t size, int timeout)
{
	if(!data)
		return -1;
	char *char_data = (char*)data;
	fds.events = POLLIN;
	int read_bytes = 0;
	int poll_res;
	while(read_bytes < (int)size)
	{
		int bytes_for_read = size - read_bytes;
		poll_res = TEMP_FAILURE_RETRY(poll(&fds, 1, timeout));
		if(poll_res <= 0)
		{
			if(poll_res == 0)
				errno = ETIMEDOUT;
			perror("Socket::SyncRecv poll");
			return -1;
		}
		if(fds.revents & POLLHUP || fds.revents & POLLERR || fds.revents & POLLNVAL)
		{
 			Disconnect();
			return 0;
		}
		int tmp_bytes;
		tmp_bytes = TEMP_FAILURE_RETRY(recv(sock, &char_data[read_bytes], 
										   bytes_for_read, MSG_NOSIGNAL));
		if(tmp_bytes == -1)
		{
			perror("Socket::SyncRecv recv");
			return -1;
		}
		if(!tmp_bytes)
		{
 			Disconnect();
			return 0;
		}
		read_bytes += tmp_bytes;
	}
	return read_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::AsyncRecv(void *data, size_t size, int timeout)
{
	if(!data)
		return -1;
	int read_bytes = 0;
	char *char_data = (char*)data;
	int tmp_bytes;
	while(read_bytes < (int)size)
	{
		int bytes_for_read = size - read_bytes;
		tmp_bytes = TEMP_FAILURE_RETRY(recv(sock, &char_data[read_bytes], 
									   bytes_for_read, MSG_NOSIGNAL));
		if(tmp_bytes == -1)
		{
			perror("Socket::AsyncRecv recv");
			return -1;
		}
		if(tmp_bytes == 0)
		{
			Disconnect();
			return 0;
		}
		read_bytes += tmp_bytes;
	}
	memcpy(data, char_data, size);
	return tmp_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::Recv(void *data, size_t size, int timeout)
{
	if(!data)
		return -1;
	if(fcntl(sock, F_GETFL) & O_NONBLOCK)
		return SyncRecv(data, size, timeout);
	if(fcntl(sock, F_GETFL) & O_ASYNC)
		return AsyncRecv(data, size, timeout);
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////

void Socket::WaitData(int timeout)
{
	fds.events = POLLIN;
	int poll_res;
	poll_res = TEMP_FAILURE_RETRY(poll(&fds, 1, timeout));
	if(poll_res <= 0)
	{
		if(poll_res == 0)
			errno = ETIMEDOUT;
		perror("Socket::SyncRecv poll");
	}
}

////////////////////////////////////////////////////////////////////////////////////

bool Socket::Connected() const
{
	return connected;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::Fd() const
{
	return sock;
}
