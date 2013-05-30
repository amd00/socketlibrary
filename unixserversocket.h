
#ifndef _UNIXSERVERSOCKET_H_
#define _UNIXSERVERSOCKET_H_

#include "serversocket.h"
#include "unixsocket.h"

////////////////////////////////////////////////////////////////////////////////////

//! \brief Интерфейс для создания Unix сервера.
/*! Интерфейс, с помощью которого можно создать UNIX сервер.
	Для этого необходимо создать наследника данного класса и определить методы
	\e Accept и \e Receiver. Запуск сервера осуществляется с помощью вызова 
	одного из трёх методов: \e Start \e AsyncStart или \e ThreadStart, описанных 
	в классе \e ServerSocket.
 */

class UnixServerSocket : public ServerSocket
{
protected:
	//! Обработка нового локального соединения.
	/*! Абстрактный метод, который будет вызываться при новом подключении к серверу.
		Метод должен быть определён в классе-потомке.

		\param sock - вновь подключённый сокет.
	*/
	virtual void Accept(UnixSocket *sock) = 0;	// Обработка новых подключений
	
	//! Обработка входящих данных.
	/*! Абстрактный метод, который будет вызываться при наличии в сокете входящих данных.
		Метод должен быть определён в классе-потомке.

		\param sock - сокет, в который пришли данные.
	 */
	virtual void Receiver(UnixSocket *sock) = 0;	// Обработка входящих данных
private:
	
	//! Создание нового сокета.
	/*! Создаёт новый объект класса \e UnixSocket по файловому дескриптору.
	
		\param fd - файловый дескриптор.
		
		\return Указатель на новый объект класса \e UnixSocket.
	 */
	UnixSocket *GetNewSocket(int fd);			// Создание нового объекта "UnixSocket"
	
	//! Обработка нового подключения.
	/*! Реализует абстрактный метод класса \e ServerSocket. Внутри метода происходит
		вызов \e Accept(UnixSocket*).
	 */
	void Accept(Socket *sock);					// Вызывает Accept(UnixSocket*)
	
	//! Обработка входящих данных.
	/*! Реализует абстрактный метод класса \e ServerSocket. Внутри метода происходит
		вызов \e Receiver(UnixSocket*).
	 */
	void Receiver(Socket *sock);					// Вызывает Receiver(UnixSocket*)
public:
	
	//! Конструктор
	/*! Создаёт новый объект класса \e UnixServerSocket
		\param path - файл для прослушивания.
	 */
	UnixServerSocket(const char *path);			// Конструктор
	
	//! Деструктор.
	virtual ~UnixServerSocket();			// Деструктор
};

////////////////////////////////////////////////////////////////////////////////////

#endif
