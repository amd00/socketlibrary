
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

bool Socket::connect(int _domain, int _type, sockaddr *_addr, socklen_t _sock_len)
{
	m_fds.fd = ::socket(_domain, _type, 0);
	if(fd() == -1)
	{
		perror("Socket::Connect socket");
		return false;
	}
	int conn_res = ::connect(fd(), _addr, _sock_len);
// Подключение к серверу
	if(conn_res == -1)
	{
		perror("Socket::Connect connect");
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
	return true;
}

////////////////////////////////////////////////////////////////////////////////////

void Socket::disconnect()
{
// Закрытие сокета
	if(fd() == -1)
		return;
	if(::close(fd()) == -1)
		perror("Socket::Disconnect close");
	m_fds.fd = -1;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::send(const void *_data, size_t _size, int _timeout, bool *_timedout)
{
	char *char_data = (char*)_data;
	m_fds.events = POLLOUT;
	int sent_bytes = 0;
	while(sent_bytes < (int)_size)
	{
		int bytes_for_send = _size - sent_bytes;
		switch(::poll(&m_fds, 1, _timeout))
		{
			case -1:
				perror("Socket::send poll");
				return -1;
			case 0:
				if(_timedout)
					*_timedout = true;
				return 0;
		}
		if(m_fds.revents & POLLHUP || m_fds.revents & POLLERR)
			return 0;
		int tmp_bytes = ::send(fd(), &char_data[sent_bytes], bytes_for_send, MSG_NOSIGNAL);
		if(tmp_bytes == -1)
		{
			perror("Socket::send send");
			return -1;
		}
		sent_bytes += tmp_bytes;
	}
	return sent_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

int Socket::recv(void *_data, size_t _size, int _timeout, bool *_timedout)
{
	char *char_data = (char*)_data;
	m_fds.events = POLLIN;
	int read_bytes = 0;
	while(read_bytes < (int)_size)
	{
		int bytes_for_read = _size - read_bytes;
		switch(::poll(&m_fds, 1, _timeout))
		{
			case -1:
				perror("Socket::recv poll");
				return -1;
			case 0:
				if(_timedout)
					*_timedout = true;
				return 0;
		}
		if(m_fds.revents & POLLHUP || m_fds.revents & POLLERR || m_fds.revents & POLLNVAL)
			return 0;
		int tmp_bytes = ::recv(fd(), &char_data[read_bytes], 
										   bytes_for_read, MSG_NOSIGNAL);
		if(tmp_bytes == -1)
		{
			perror("Socket::recv recv");
			return -1;
		}
		if(!tmp_bytes)
			return 0;
		read_bytes += tmp_bytes;
	}
	return read_bytes;
}

////////////////////////////////////////////////////////////////////////////////////

void Socket::waitData(int _timeout)
{
	m_fds.events = POLLIN;
	if(::poll(&m_fds, 1, _timeout) <= 0)
		perror("Socket::SyncRecv poll");
}
