
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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <sys/poll.h>

class Socket
{
	friend class ServerSocket;

private:
	pollfd m_fds;

protected:
	bool connect(int _domain, int _type, sockaddr *_addr, socklen_t _sl);
	Socket(int _fd) { m_fds.fd = _fd; }

public:
	Socket() {}
	virtual ~Socket() { disconnect(); }
	virtual bool connect() = 0;
	int send(const void *data, size_t data_size, int timeout = -1, bool *_timedout = 0);
	int recv(void *data, size_t data_size, int timeout = -1, bool *_timedout = 0);
	void waitData(int timeout = -1);
	void disconnect();
	virtual const char *address() const = 0;
	int fd() const { return m_fds.fd; }
};

#endif

