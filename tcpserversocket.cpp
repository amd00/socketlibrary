
#include "tcpserversocket.h"

////////////////////////////////////////////////////////////////////////////////////

TcpServerSocket::TcpServerSocket(int port) : ServerSocket()
{
// Создание серверного сокета
	((sockaddr_in*)&addr) -> sin_family = AF_INET;
	((sockaddr_in*)&addr) -> sin_port = htons(port);
	((sockaddr_in*)&addr) -> sin_addr.s_addr = INADDR_ANY;
	listener = socket(PF_INET, SOCK_STREAM, 0);
}

////////////////////////////////////////////////////////////////////////////////////

TcpServerSocket::~TcpServerSocket()
{
}

////////////////////////////////////////////////////////////////////////////////////

TcpSocket *TcpServerSocket::GetNewSocket(int fd)
{
	TcpSocket *tmp_socket = new TcpSocket(fd);
	return tmp_socket;
}

////////////////////////////////////////////////////////////////////////////////////

void TcpServerSocket::Accept(Socket *sock)
{
	Accept((TcpSocket*)sock);
}

////////////////////////////////////////////////////////////////////////////////////

void TcpServerSocket::Receiver(Socket *sock)
{
	Receiver((TcpSocket*)sock);
}

////////////////////////////////////////////////////////////////////////////////////

