
#ifndef _UNIXSOCKET_H_
#define _UNIXSOCKET_H_

#include <sys/un.h>

#include "socket.h"

////////////////////////////////////////////////////////////////////////////////////

//! \brief Класс, реализующий взаимодействие через UNIX сокет.
/*! Класс предоставляет интерфейс для взаимодействия через UNIX сокеты.
	С помощью обекта класса \e UnixSocket можно отправлять и принимать данные через
	Unix-сокет.
	Например эхо-клиент:

	UnixSocket sock("/tmp/tmp_sock");\n
	char msg[] = "Hello, world!";\n
	int msg_len = strlen(msg);\n
	if(!sock.Connect())\n
	...\n
	sock.Send(msg, msg_len);\n
	memset(msg, 0, msg_len + 1);\n
	sock.Recv(msg, msg_len);\n
 */
class UnixSocket : public Socket
{
	/*! \sa UnixServerSocket
	 */
	friend class UnixServerSocket;
private:
	//! Структура типа \e sockaddr_un.
	/*! Структура, описывающая параметры сокета.
	 */
	sockaddr_un addr;
	//! Конструктор.
	/*! Создаёт объект класса \e UnixSocket, используя файловый дескриптор.
	
		\param fd - файловый дескриптор.
	 */
	UnixSocket(int fd);
public:
	//! Конструктор.
	/*! Создаёт объект класса \e UnixSocket, используя адрес и порт удалённой машины.
	
		\param path - путь к серверу на локальной машине.
	 */
	UnixSocket(const char *path);
	//! Деструктор.
	~UnixSocket();
	//! Подключение к серверу.
	/*! Метод для подключения к серверу. Вызывает метод 
		\e Connect(int, int, sockaddr*, socklen_t) родительского класса \e Socket.
	
		\return Возвращает \b true - в случае успешного подключения, \b false - в случае неудачи.
	
		\sa Socket::Connect(int, int, sockaddr*, socklen_t)
	 */
	bool Connect();
	//! Путь к сокету в файловой системе.
	/*! Абсолютный путь к сокету в файловой системе. Определяется из структуры \e addr.
	
		\return Возвращает строку, содержащую путь к сокету.
	 */
	const char *Address();
};

////////////////////////////////////////////////////////////////////////////////////

#endif
