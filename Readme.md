# Web Chat Application - NADDYA

*Клиент–серверное приложение чата, реализующее регистрацию пользователей, вход, создание чатов, присоединение и обмен сообщениями.*

**Сервер:** C++ (Crow + SQLite). 
**Клиент:** HTML/JS.

## Как собрать

### Windows
```bash
build.bat
```

### Linux/macOS
Установка зависимостей:
```bash
sudo apt install g++ libsqlite3-dev make
```

Сборка:
```bash
mkdir build
cd build
g++ -std=c++17 -O2 -pthread \
  -I"../Crow/include" \
  -I"../asio/asio/include" \
  -I"../backend/src" \
  ../backend/src/*.cpp \
  -lsqlite3 \
  -o web_chat_server
```

## Как запустить

### Windows
```bash
run_server.bat
```

### Linux/macOS
```bash
./web_chat_server
```

После запуска сервер доступен на:
```
http://localhost:8080
```

## Формат входных данных / API

### Регистрация
```http
POST /api/register
{
  "username": "name",
  "password": "pass"
}
```

### Вход
```http
POST /api/login
{
  "username": "name",
  "password": "pass"
}
```

Ответ содержит `session_token`.

### Создание чата
```http
POST /api/chats/create_with_privacy
{
  "chat_name": "Room",
  "is_public": true
}
```

### Присоединение
```http
POST /api/chats/join
{
  "chat_id": 123
}
```

### Отправка сообщения
```http
POST /api/messages
{
  "chat_id": 123,
  "content": "hello"
}
```

### Получение сообщений
```http
GET /api/chats/<chat_id>/messages?limit=50
```

## Примеры запуска (curl)

Регистрация:
```bash
curl -X POST http://localhost:8080/api/register \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"test\",\"password\":\"123\"}"
```

Вход:
```bash
curl -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"test\",\"password\":\"123\"}"
```

Создание публичного чата:
```bash
curl -X POST http://localhost:8080/api/chats/create_with_privacy \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <token>" \
  -d "{\"chat_name\":\"General\",\"is_public\":true}"
```
