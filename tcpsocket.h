
#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

 #include <netinet/in.h>

#include "socket.h"

////////////////////////////////////////////////////////////////////////////////////

//! \brief Класс, реализующий взаимодействие через TCP сокет.
/*! Класс предоставляет интерфейс для взаимодействия через протокол TCP/IP.
	С помощью обекта класса \e TcpSocket можно отправлять и принимать данные через
	TCP-сокет.
	Например эхо-клиент:

	TcpSocket sock("192.168.1.100", 6543);\n
	char msg[] = "Hello, world!";\n
	int msg_len = strlen(msg);\n
	if(!sock.Connect())\n
	...\n
	sock.Send(msg, msg_len);\n
	memset(msg, 0, msg_len + 1);\n
	sock.Recv(msg, msg_len);\n
*/
class TcpSocket : public Socket
{
	/*! \sa TcpServerSocket
	*/
	friend class TcpServerSocket;
private:
	
	//! Структура типа \e sockaddr_in.
	/*! Структура, описывающая параметры сокета.
	*/
	sockaddr_in addr;
	
	//! Конструктор.
	/*! Создаёт объект класса \e TcpSocket, используя файловый дескриптор.
	
		\param fd - файловый дескриптор.
	*/
	TcpSocket(int fd);
public:
	
	//! Конструктор.
	/*! Создаёт объект класса \e TcpSocket, используя адрес и порт удалённой машины.
	
		\param ip - ip-адрес удалённой машины.
		\param port - порт для подключения к удалённой машине.
	*/
	TcpSocket(const char *ip, int port);
	
	//! Деструктор.
	~TcpSocket();
	
	//! Подключение к серверу.
	/*! Метод для подключения к серверу. Вызывает метод 
		\e Connect(int, int, sockaddr*, socklen_t) родительского класса \e Socket.
	
		\return Возвращает \b true - в случае успешного подключения, \b false - в случае неудачи.
	
		\sa Socket::Connect(int, int, sockaddr*, socklen_t)
	 */
	bool Connect();
	
	//! Адрес удалённой машины.
	/*! ip-адрес удалённой машины. Определяется с помощью вызова функции \e inet_ntoa из
		структуры \e addr.
	
		\return Возвращает строку, содержащую ip-адрес удалённой машины.
	 */
	const char *Address();
	
	//! Порт для подключения.
	/*! Порт для подключения к удалённой машине. Определяется с помощью вызова 
		функции \e ntohs из	структуры \e addr.
	
		\return Возвращает номер порта для подключения к удалённой машине.
	 */
	int Port() const;
};

////////////////////////////////////////////////////////////////////////////////////

#endif
