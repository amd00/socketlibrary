
cmake_minimum_required(VERSION 2.8)	 # Проверка версии CMake.
                                        # Если версия установленой программы
                                        # старее указаной, произайдёт аварийный выход.

project(socketlibrary)			# Название проекта

# set(SOURCE_EXE main.cpp)		# Установка переменной со списком исходников для исполняемого файла

set(SOURCE_LIB serversocket.cpp  socket.cpp  tcpserversocket.cpp  tcpsocket.cpp  unixserversocket.cpp  unixsocket.cpp)			# Тоже самое, но для библиотеки
set(HEADER_LIB serversocket.h  socket.h  tcpserversocket.h  tcpsocket.h  unixserversocket.h  unixsocket.h)			# Тоже самое, но для библиотеки

add_library(socketlibraryS STATIC ${SOURCE_LIB})	# Создание статической библиотеки с именем foo
add_library(socketlibrary SHARED ${SOURCE_LIB})	# Создание динамической библиотеки с именем foo

set_target_properties(socketlibrary  PROPERTIES
	  VERSION 1.0.0
	  SOVERSION 1.0
	)
	
INSTALL(TARGETS socketlibraryS socketlibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

INSTALL(FILES ${HEADER_LIB} 
	DESTINATION include)

# add_executable(main ${SOURCE_EXE})	# Создает исполняемый файл с именем main

# target_link_libraries(main foo)		# Линковка программы с библиотекой