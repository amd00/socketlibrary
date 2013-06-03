
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

#ifndef _UNIXSOCKET_H_
#define _UNIXSOCKET_H_

#include <sys/un.h>

#include "socket.h"

class UnixSocket : public Socket
{
	friend class UnixServerSocket;

private:
	sockaddr_un m_addr;

public:
	UnixSocket(const char *_path);
	~UnixSocket() {}
	
	bool connect() { return Socket::connect(PF_UNIX, SOCK_STREAM, (sockaddr*)&m_addr, sizeof(m_addr)); }
	const char *address() const { return m_addr.sun_path; }
	
private:
	UnixSocket(int _fd) : Socket(_fd) {}
};

#endif
