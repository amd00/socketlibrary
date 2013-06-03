
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

#ifndef _TCPSERVERSOCKET_H_
#define _TCPSERVERSOCKET_H_

#include "serversocket.h"
#include "tcpsocket.h"

class TcpServerSocket : public ServerSocket
{
public:
	TcpServerSocket(int _port);
	virtual ~TcpServerSocket() {}
	
protected:
	virtual void accept(TcpSocket *_sock) = 0;
	virtual void receiver(TcpSocket *_sock) = 0;

private:
	TcpSocket *getNewSocket(int _fd) const { return new TcpSocket(_fd); }
	void accept(Socket *_sock) { accept((TcpSocket*)_sock); }
	void receiver(Socket *_sock) { receiver((TcpSocket*)_sock); }
};

#endif
