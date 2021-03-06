
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

#ifndef _UNIXSERVERSOCKET_H_
#define _UNIXSERVERSOCKET_H_

#include "serversocket.h"
#include "unixsocket.h"

class UnixServerSocket : public ServerSocket
{
protected:
	virtual void accept(UnixSocket *_sock) = 0;	// Обработка новых подключений
	virtual void receiver(UnixSocket *_sock) = 0;	// Обработка входящих данных

private:
	UnixSocket *getNewSocket(int _fd, sockaddr *_addr) { return new UnixSocket(_fd, _addr); }
	void accept(Socket *_sock) { accept((UnixSocket*)_sock); }
	void receiver(Socket *_sock) { receiver((UnixSocket*)_sock); }

public:
	UnixServerSocket(const char *_path);			// Конструктор
	virtual ~UnixServerSocket() {}
};

#endif
