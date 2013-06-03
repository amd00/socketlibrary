
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

#include <string.h>
#include <stdio.h>

#include "tcpsocket.h"

////////////////////////////////////////////////////////////////////////////////////

TcpSocket::TcpSocket(const char *_ip_addr, int _port) : Socket()
{
	::memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = ::htons(_port);
	if(!::inet_aton(_ip_addr ? _ip_addr : "127.0.0.1", &m_addr.sin_addr))
		::fprintf(stderr, "Incorrect address \"%s\"\n", _ip_addr);
}

TcpSocket::TcpSocket(int _fd) : Socket(_fd)
{
	::memset(&m_addr, 0, sizeof(m_addr));
	socklen_t addr_len = sizeof(m_addr);
	if(::getpeername(_fd, (sockaddr*)&m_addr, &addr_len))
		::perror("TcpSocket::TcpSocket getpeername");
}
