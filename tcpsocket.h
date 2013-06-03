
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

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <netinet/in.h>
#include <arpa/inet.h>

#include "socket.h"

class TcpSocket : public Socket
{
	friend class TcpServerSocket;

private:
	sockaddr_in m_addr;

public:
	TcpSocket(const char *_ip, int _port);
	~TcpSocket() {}

	bool connect() { return Socket::connect(PF_INET, SOCK_STREAM, (sockaddr*)&m_addr, sizeof(m_addr)); }
	const char *address() const { return ::inet_ntoa(m_addr.sin_addr); }
	int port() const { return ::ntohs(m_addr.sin_port); }
	
private:
	TcpSocket(int _fd);
};

#endif
