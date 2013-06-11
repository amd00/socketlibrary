
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

#include "unixsocket.h"

UnixSocket::UnixSocket(int _fd, sockaddr *_addr) : Socket(_fd)
{
	m_addr = (sockaddr_un*)_addr;
}

UnixSocket::UnixSocket(const char *_path) : Socket(), m_addr(new sockaddr_un)
{
	::memset(m_addr, 0, sizeof(sockaddr_un));
	m_addr -> sun_family = AF_UNIX;
	::strcpy(m_addr -> sun_path, _path ? _path : "");
}
