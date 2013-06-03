
#include "tcpserversocket.h"

TcpServerSocket::TcpServerSocket(int _port) : ServerSocket()
{
// Создание серверного сокета
	((sockaddr_in*)&m_addr) -> sin_family = AF_INET;
	((sockaddr_in*)&m_addr) -> sin_port = htons(_port);
	((sockaddr_in*)&m_addr) -> sin_addr.s_addr = INADDR_ANY;
	m_listener = socket(PF_INET, SOCK_STREAM, 0);
}
