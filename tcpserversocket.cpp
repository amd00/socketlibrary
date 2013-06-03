
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

#include "tcpserversocket.h"

TcpServerSocket::TcpServerSocket(int _port) : ServerSocket()
{
// Создание серверного сокета
	((sockaddr_in*)&m_addr) -> sin_family = AF_INET;
	((sockaddr_in*)&m_addr) -> sin_port = ::htons(_port);
	((sockaddr_in*)&m_addr) -> sin_addr.s_addr = INADDR_ANY;
	m_listener = ::socket(PF_INET, SOCK_STREAM, 0);
}
