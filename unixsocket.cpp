
#include "unixsocket.h"

UnixSocket::UnixSocket(const char *_path) : Socket()
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sun_family = AF_UNIX;
	strcpy(m_addr.sun_path, _path ? _path : "");
}
