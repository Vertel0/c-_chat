cmake_minimum_required(VERSION 3.15)
project(WebChatApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для MinGW
if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O2")
endif()

# Поиск SQLite3
find_package(PkgConfig REQUIRED)
find_library(SQLITE3_LIBRARIES NAMES sqlite3)

if(NOT SQLITE3_LIBRARIES)
    message(WARNING "SQLite3 not found, trying to find sqlite3.dll")
    # Попробуем найти в common locations
    find_library(SQLITE3_LIBRARIES 
        NAMES sqlite3
        PATHS 
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            C:/Windows/System32
            C:/MinGW/lib
    )
endif()

if(SQLITE3_LIBRARIES)
    message(STATUS "Found SQLite3: ${SQLITE3_LIBRARIES}")
else()
    message(FATAL_ERROR "SQLite3 library not found! Please install sqlite3")
endif()

# Включаем пути
include_directories(
    ${CMAKE_SOURCE_DIR}/Crow
    ${CMAKE_SOURCE_DIR}/asio/asio/include
)

# Список исходных файлов (ДОБАВЛЕН database.cpp)
set(SOURCES
    backend/src/main.cpp
    backend/src/webserver.cpp
    backend/src/chat_manager.cpp
    backend/src/user.cpp
    backend/src/chat.cpp
    backend/src/message.cpp
    backend/src/database.cpp      # ← ДОБАВЛЕНО
)

# Создаем исполняемый файл
add_executable(web_chat_server ${SOURCES})

# Линкуем библиотеки для Windows
if(WIN32)
    target_link_libraries(web_chat_server 
        ws2_32 
        wsock32 
        crypt32 
        ${SQLITE3_LIBRARIES}     # ← ДОБАВЛЕНО
    )
    
    target_compile_definitions(web_chat_server PRIVATE
        _WIN32_WINNT=0x0601
        CROW_STATIC_DEFINE
        ASIO_STANDALONE
    )
else()
    # Для Linux/Mac
    target_link_libraries(web_chat_server 
        pthread 
        ${SQLITE3_LIBRARIES}     # ← ДОБАВЛЕНО
    )
endif()

# Копирование статических файлов
configure_file(backend/templates/index.html ${CMAKE_CURRENT_BINARY_DIR}/templates/index.html COPYONLY)
file(COPY backend/static DESTINATION ${CMAKE_CURRENT_BINARY_DIR})