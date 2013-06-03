
#include "unixserversocket.h"

UnixServerSocket::UnixServerSocket(const char *_path) : ServerSocket()
{
// Создание серверного сокета
	((sockaddr_un*)&m_addr) -> sun_family = AF_UNIX;
	strcpy(((sockaddr_un*)&m_addr) -> sun_path, _path ? _path : "");
	m_listener = socket(PF_UNIX, SOCK_STREAM, 0);
}
