
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

Socket::Socket() : m_connected(false)
{
}

////////////////////////////////////////////////////////////////////////////////////

Socket::Socket(int _fd) : m_last_error(0), m_connected(true)
{
	m_fds.fd = _fd;
// Определение типа сокета (синхронный или асинхронный)
	int sock_flags = ::fcntl(fd(), F_GETFL);
	if(sock_flags == -1)
		perror("Socket::Socket fcntl");
	if(!(sock_flags & O_ASYNC))
	{
		if(::fcntl(fd(), F_SETFL, O_NONBLOCK) == -1)
			perror("Socket::Socket fcntl");
	}
}

////////////////////////////////////////////////////////////////////////////////////

Socket::~Socket()
{
	disconnect();
}

////////////////////////////////////////////////////////////////////////////////////

bool Socket::connect(int _domain, int _type, sockaddr *_addr, socklen_t _sock_len)
{
	if(m_connected)
		return false;
	m_fds.fd = ::socket(_domain, _type, 0);
	if(fd() == -1)
	{
		perror("Socket::Connect socket");
		return false;
	}
	int conn_res = TEMP_FAILURE_RETRY(::connect(fd(), _addr, _sock_len));
// Подключение к серверу
	if(conn_res == -1)
	{
		perror("Socket::Connect connect");
		if(errno == EISCONN)
		{
			m_connected = true;
			return false;
		}
		disconnect();
		return false;
	}
// Создание неблокируемого сокета
	if(::fcntl(fd(), F_SETFL, O_NONBLOCK) == -1)
	{
		perror("Socket::Connect fcntl");
 		disconnect();
		return false;
	}
	m_connected = true;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////

void Socket::disconnect()
{
// Закрытие сокета
	m_connected = false;
	if(fd() == -1)
		return;
	if(::close(fd()) == -1)
		perror("Socket::Disconnect close");
	m_fds.fd = -1;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::syncSend(const void *_data, size_t _size, int _timeout)
{
// Синхронная посылка данных
	if(!_data)
		return -1;
	char *char_data = (char*)_data;
	m_fds.events = POLLOUT;
	int sent_bytes = 0;
	int poll_res;
	while(sent_bytes < (int)_size)
	{
		int bytes_for_send = _size - sent_bytes;
		poll_res = TEMP_FAILURE_RETRY(::poll(&m_fds, 1, _timeout));
		if(poll_res <= 0)
		{
			if(poll_res == 0)
				errno = ETIMEDOUT;
			perror("Socket::SyncSend poll");
			return -1;
		}
		if(m_fds.revents & POLLHUP || m_fds.revents & POLLERR)
		{
 			disconnect();
			return 0;
		}
		int tmp_bytes = TEMP_FAILURE_RETRY(::send(fd(), &char_data[sent_bytes], bytes_for_send, MSG_NOSIGNAL));
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

int Socket::asyncSend(const void *_data, size_t _size, int _timeout)
{
	int tmp_bytes = TEMP_FAILURE_RETRY(::send(fd(), _data, _size, MSG_NOSIGNAL));
	if(tmp_bytes == -1)
	{
		perror("Socket::AsyncSend send");
		if(errno == EPIPE)
		{
			disconnect();
			tmp_bytes = 0;
		}
	}
	return tmp_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::send(const void *_data, size_t _size, int _timeout)
{
	if(!_data)
		return -1;
	if(::fcntl(fd(), F_GETFL) & O_ASYNC)
		return asyncSend(_data, _size, _timeout);
	else if(::fcntl(fd(), F_GETFL) & O_NONBLOCK)
		return syncSend(_data, _size, _timeout);
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::syncRecv(void *_data, size_t _size, int _timeout)
{
	if(!_data)
		return -1;
	char *char_data = (char*)_data;
	m_fds.events = POLLIN;
	int read_bytes = 0;
	int poll_res;
	while(read_bytes < (int)_size)
	{
		int bytes_for_read = _size - read_bytes;
		poll_res = TEMP_FAILURE_RETRY(::poll(&m_fds, 1, _timeout));
		if(poll_res <= 0)
		{
			if(poll_res == 0)
				errno = ETIMEDOUT;
			perror("Socket::SyncRecv poll");
			return -1;
		}
		if(m_fds.revents & POLLHUP || m_fds.revents & POLLERR || m_fds.revents & POLLNVAL)
		{
 			disconnect();
			return 0;
		}
		int tmp_bytes;
		tmp_bytes = TEMP_FAILURE_RETRY(::recv(fd(), &char_data[read_bytes], 
										   bytes_for_read, MSG_NOSIGNAL));
		if(tmp_bytes == -1)
		{
			perror("Socket::SyncRecv recv");
			return -1;
		}
		if(!tmp_bytes)
		{
 			disconnect();
			return 0;
		}
		read_bytes += tmp_bytes;
	}
	return read_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::asyncRecv(void *_data, size_t _size, int _timeout)
{
	if(!_data)
		return -1;
	int read_bytes = 0;
	char *char_data = (char*)_data;
	int tmp_bytes;
	while(read_bytes < (int)_size)
	{
		int bytes_for_read = _size - read_bytes;
		tmp_bytes = TEMP_FAILURE_RETRY(::recv(fd(), &char_data[read_bytes], 
									   bytes_for_read, MSG_NOSIGNAL));
		if(tmp_bytes == -1)
		{
			perror("Socket::AsyncRecv recv");
			return -1;
		}
		if(tmp_bytes == 0)
		{
			disconnect();
			return 0;
		}
		read_bytes += tmp_bytes;
	}
	::memcpy(_data, char_data, _size);
	return tmp_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::recv(void *_data, size_t _size, int _timeout)
{
	if(!_data)
		return -1;
	if(::fcntl(fd(), F_GETFL) & O_ASYNC)
		return asyncRecv(_data, _size, _timeout);
	if(::fcntl(fd(), F_GETFL) & O_NONBLOCK)
		return syncRecv(_data, _size, _timeout);
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////

void Socket::waitData(int _timeout)
{
	m_fds.events = POLLIN;
	int poll_res;
	poll_res = TEMP_FAILURE_RETRY(::poll(&m_fds, 1, _timeout));
	if(poll_res <= 0)
	{
		if(poll_res == 0)
			errno = ETIMEDOUT;
		perror("Socket::SyncRecv poll");
	}
}
