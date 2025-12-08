#include "database.h"
#include <iostream>
#include <sstream>
#include <chrono>

Database::Database(const std::string& path) : db_path(path), db(nullptr) {}

Database::~Database() {
    close();
}

bool Database::initialize() {
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Create tables
    const char* create_tables_sql = 
    "CREATE TABLE IF NOT EXISTS users ("
    "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "username TEXT UNIQUE NOT NULL,"
    "password_hash TEXT NOT NULL,"
    "email TEXT,"
    "session_token TEXT,"
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    ");"
    
    "CREATE TABLE IF NOT EXISTS chats ("
    "chat_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "chat_name TEXT NOT NULL,"
    "chat_type TEXT DEFAULT 'group',"
    "created_by INTEGER,"
    "is_public INTEGER DEFAULT 1,"
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY (created_by) REFERENCES users(user_id)"
    ");"
    
    "CREATE TABLE IF NOT EXISTS chat_whitelist ("
    "chat_id INTEGER,"
    "user_id INTEGER,"
    "invited_by INTEGER,"
    "invited_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "PRIMARY KEY (chat_id, user_id),"
    "FOREIGN KEY (chat_id) REFERENCES chats(chat_id),"
    "FOREIGN KEY (user_id) REFERENCES users(user_id),"
    "FOREIGN KEY (invited_by) REFERENCES users(user_id)"
    ");"  // ← ТОЛЬКО ОДНА ТОЧКА С ЗАПЯТОЙ!
    
    "CREATE TABLE IF NOT EXISTS chat_members ("
    "user_id INTEGER,"
    "chat_id INTEGER,"
    "joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "PRIMARY KEY (user_id, chat_id),"
    "FOREIGN KEY (user_id) REFERENCES users(user_id),"
    "FOREIGN KEY (chat_id) REFERENCES chats(chat_id)"
    ");"
    
    "CREATE TABLE IF NOT EXISTS messages ("
    "message_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "chat_id INTEGER NOT NULL,"
    "sender_id INTEGER NOT NULL,"
    "sender_name TEXT NOT NULL,"
    "content TEXT NOT NULL,"
    "message_type TEXT DEFAULT 'text',"
    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "FOREIGN KEY (chat_id) REFERENCES chats(chat_id),"
    "FOREIGN KEY (sender_id) REFERENCES users(user_id)"
    ");";
    
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, create_tables_sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    std::cout << "Database initialized successfully" << std::endl;
    return true;
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

// User operations
bool Database::createUser(const std::string& username, const std::string& password_hash, const std::string& email) {
    const char* sql = "INSERT INTO users (username, password_hash, email) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

User* Database::getUserByUsername(const std::string& username) const {
    const char* sql = "SELECT user_id, username, password_hash, email, session_token FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем значения из базы данных
        int user_id = sqlite3_column_int(stmt, 0);
        const unsigned char* username_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* password_hash_ptr = sqlite3_column_text(stmt, 2);
        const unsigned char* email_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* session_token_ptr = sqlite3_column_text(stmt, 4);
        
        // Преобразуем в std::string
        std::string username_str = username_ptr ? reinterpret_cast<const char*>(username_ptr) : "";
        std::string password_hash_str = password_hash_ptr ? reinterpret_cast<const char*>(password_hash_ptr) : "";
        std::string email_str = email_ptr ? reinterpret_cast<const char*>(email_ptr) : "";
        std::string session_token_str = session_token_ptr ? reinterpret_cast<const char*>(session_token_ptr) : "";
        
        // Создаем пользователя с помощью конструктора для БД
        user = new User(user_id, username_str, password_hash_str, email_str, session_token_str);
    }
    
    sqlite3_finalize(stmt);
    return user;
}

User* Database::getUserById(int user_id) const {
    const char* sql = "SELECT user_id, username, password_hash, email, session_token FROM users WHERE user_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем значения из базы данных
        int db_user_id = sqlite3_column_int(stmt, 0);
        const unsigned char* username_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* password_hash_ptr = sqlite3_column_text(stmt, 2);
        const unsigned char* email_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* session_token_ptr = sqlite3_column_text(stmt, 4);
        
        // Преобразуем в std::string
        std::string username_str = username_ptr ? reinterpret_cast<const char*>(username_ptr) : "";
        std::string password_hash_str = password_hash_ptr ? reinterpret_cast<const char*>(password_hash_ptr) : "";
        std::string email_str = email_ptr ? reinterpret_cast<const char*>(email_ptr) : "";
        std::string session_token_str = session_token_ptr ? reinterpret_cast<const char*>(session_token_ptr) : "";
        
        // Создаем пользователя с помощью конструктора для БД
        user = new User(db_user_id, username_str, password_hash_str, email_str, session_token_str);
    }
    
    sqlite3_finalize(stmt);
    return user;
}

bool Database::updateUserSession(int user_id, const std::string& session_token) {
    const char* sql = "UPDATE users SET session_token = ? WHERE user_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, session_token.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

// Chat operations
int Database::createChat(const std::string& chat_name, int creator_id, const std::string& type, bool is_public) {
    const char* sql = "INSERT INTO chats (chat_name, created_by, chat_type, is_public) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, chat_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, creator_id);
    sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, is_public ? 1 : 0);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int chat_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    // Add creator to chat members
    addUserToChat(creator_id, chat_id);
    
    // If private chat, add creator to whitelist
    if (!is_public) {
        addToWhitelist(chat_id, creator_id, creator_id);
    }
    
    return chat_id;
}

bool Database::addToWhitelist(int chat_id, int user_id, int invited_by) {
    const char* sql = "INSERT OR REPLACE INTO chat_whitelist (chat_id, user_id, invited_by) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_int(stmt, 3, invited_by);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::isUserInWhitelist(int user_id, int chat_id) const {
    const char* sql = "SELECT 1 FROM chat_whitelist WHERE user_id = ? AND chat_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, chat_id);
    
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return exists;
}

// Обновите метод getChatById для загрузки whitelist
Chat* Database::getChatById(int chat_id) const{
    const char* sql = "SELECT chat_id, chat_name, chat_type, created_by, is_public FROM chats WHERE chat_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_int(stmt, 1, chat_id);
    
    Chat* chat = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int db_chat_id = sqlite3_column_int(stmt, 0);
        const unsigned char* chat_name_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* chat_type_ptr = sqlite3_column_text(stmt, 2);
        int created_by = sqlite3_column_int(stmt, 3);
        bool is_public = sqlite3_column_int(stmt, 4) == 1;
        
        std::string chat_name_str = chat_name_ptr ? reinterpret_cast<const char*>(chat_name_ptr) : "";
        std::string chat_type_str = chat_type_ptr ? reinterpret_cast<const char*>(chat_type_ptr) : "group";
        
        chat = new Chat(chat_name_str, created_by, chat_type_str, is_public);
        chat->chat_id = db_chat_id;
        
        // Load members
        const char* members_sql = "SELECT user_id FROM chat_members WHERE chat_id = ?";
        sqlite3_stmt* members_stmt;
        if (sqlite3_prepare_v2(db, members_sql, -1, &members_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(members_stmt, 1, chat->chat_id);
            while (sqlite3_step(members_stmt) == SQLITE_ROW) {
                chat->member_ids.push_back(sqlite3_column_int(members_stmt, 0));
            }
            sqlite3_finalize(members_stmt);
        }
        
        // Load whitelist for private chats
        if (!is_public) {
            const char* whitelist_sql = "SELECT user_id FROM chat_whitelist WHERE chat_id = ?";
            sqlite3_stmt* whitelist_stmt;
            if (sqlite3_prepare_v2(db, whitelist_sql, -1, &whitelist_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(whitelist_stmt, 1, chat->chat_id);
                while (sqlite3_step(whitelist_stmt) == SQLITE_ROW) {
                    chat->whitelist_ids.push_back(sqlite3_column_int(whitelist_stmt, 0));
                }
                sqlite3_finalize(whitelist_stmt);
            }
        }
    }
    
    sqlite3_finalize(stmt);
    return chat;
}

std::vector<Chat> Database::getUserChats(int user_id) const{
    std::vector<Chat> chats;
    
    const char* sql = 
        "SELECT c.chat_id, c.chat_name, c.chat_type, c.created_by, c.is_public "
        "FROM chats c "
        "JOIN chat_members cm ON c.chat_id = cm.chat_id "
        "WHERE cm.user_id = ? "
        "ORDER BY c.chat_id DESC";
    
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return chats;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем значения из базы данных
        int chat_id = sqlite3_column_int(stmt, 0);
        const unsigned char* chat_name_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* chat_type_ptr = sqlite3_column_text(stmt, 2);
        int created_by = sqlite3_column_int(stmt, 3);
        
        // Преобразуем в std::string
        std::string chat_name_str = chat_name_ptr ? reinterpret_cast<const char*>(chat_name_ptr) : "";
        std::string chat_type_str = chat_type_ptr ? reinterpret_cast<const char*>(chat_type_ptr) : "group";
        
        Chat chat(chat_name_str, created_by, chat_type_str);
        chat.chat_id = chat_id;
        
        // Load members for this chat
        const char* members_sql = "SELECT user_id FROM chat_members WHERE chat_id = ?";
        sqlite3_stmt* members_stmt;
        if (sqlite3_prepare_v2(db, members_sql, -1, &members_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(members_stmt, 1, chat.chat_id);
            while (sqlite3_step(members_stmt) == SQLITE_ROW) {
                chat.member_ids.push_back(sqlite3_column_int(members_stmt, 0));
            }
            sqlite3_finalize(members_stmt);
        }
        
        chats.push_back(chat);
    }
    
    sqlite3_finalize(stmt);
    return chats;
}

std::vector<Chat> Database::getAllChats() const {
    std::vector<Chat> chats;
    
    const char* sql = "SELECT chat_id, chat_name, chat_type, created_by FROM chats ORDER BY chat_id DESC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return chats;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем значения из базы данных
        int chat_id = sqlite3_column_int(stmt, 0);
        const unsigned char* chat_name_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* chat_type_ptr = sqlite3_column_text(stmt, 2);
        int created_by = sqlite3_column_int(stmt, 3);
        
        // Преобразуем в std::string
        std::string chat_name_str = chat_name_ptr ? reinterpret_cast<const char*>(chat_name_ptr) : "";
        std::string chat_type_str = chat_type_ptr ? reinterpret_cast<const char*>(chat_type_ptr) : "group";
        
        Chat chat(chat_name_str, created_by, chat_type_str);
        chat.chat_id = chat_id;
        
        // Load members
        const char* members_sql = "SELECT user_id FROM chat_members WHERE chat_id = ?";
        sqlite3_stmt* members_stmt;
        if (sqlite3_prepare_v2(db, members_sql, -1, &members_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(members_stmt, 1, chat.chat_id);
            while (sqlite3_step(members_stmt) == SQLITE_ROW) {
                chat.member_ids.push_back(sqlite3_column_int(members_stmt, 0));
            }
            sqlite3_finalize(members_stmt);
        }
        
        chats.push_back(chat);
    }
    
    sqlite3_finalize(stmt);
    return chats;
}

// Message operations
bool Database::addMessage(int chat_id, int sender_id, const std::string& content, const std::string& type) {
    // Get sender username
    User* sender = getUserById(sender_id);
    if (!sender) return false;
    
    const char* sql = "INSERT INTO messages (chat_id, sender_id, sender_name, content, message_type) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        delete sender;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_int(stmt, 2, sender_id);
    sqlite3_bind_text(stmt, 3, sender->username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, type.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    delete sender;
    
    return success;
}

std::vector<Message> Database::getChatMessages(int chat_id, int limit) const{
    std::vector<Message> messages;
    
    const char* sql = 
        "SELECT message_id, chat_id, sender_id, sender_name, content, message_type, timestamp "
        "FROM messages WHERE chat_id = ? ORDER BY message_id ASC LIMIT ?";
    
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return messages;
    }
    
    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_int(stmt, 2, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем значения из базы данных
        int message_id = sqlite3_column_int(stmt, 0);
        int db_chat_id = sqlite3_column_int(stmt, 1);
        int sender_id = sqlite3_column_int(stmt, 2);
        const unsigned char* sender_name_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* content_ptr = sqlite3_column_text(stmt, 4);
        const unsigned char* message_type_ptr = sqlite3_column_text(stmt, 5);
        const unsigned char* timestamp_ptr = sqlite3_column_text(stmt, 6);
        
        // Преобразуем в std::string
        std::string sender_name_str = sender_name_ptr ? reinterpret_cast<const char*>(sender_name_ptr) : "";
        std::string content_str = content_ptr ? reinterpret_cast<const char*>(content_ptr) : "";
        std::string message_type_str = message_type_ptr ? reinterpret_cast<const char*>(message_type_ptr) : "text";
        std::string timestamp_str = timestamp_ptr ? reinterpret_cast<const char*>(timestamp_ptr) : "";
        
        Message msg(message_id, db_chat_id, sender_id, sender_name_str, content_str, message_type_str);
        msg.timestamp = timestamp_str;
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

// Membership operations
// Улучшенный метод addUserToChat в database.cpp:

bool Database::addUserToChat(int user_id, int chat_id) {
    // Сначала проверяем существование пользователя и чата
    User* user = getUserById(user_id);
    if (!user) {
        std::cerr << "ERROR in addUserToChat: User " << user_id << " not found!" << std::endl;
        return false;
    }
    delete user;
    
    // Проверяем существование чата
    const char* check_chat_sql = "SELECT 1 FROM chats WHERE chat_id = ?";
    sqlite3_stmt* check_stmt;
    
    if (sqlite3_prepare_v2(db, check_chat_sql, -1, &check_stmt, nullptr) != SQLITE_OK) {
        std::cerr << "ERROR in addUserToChat: Failed to prepare check statement" << std::endl;
        return false;
    }
    
    sqlite3_bind_int(check_stmt, 1, chat_id);
    bool chat_exists = (sqlite3_step(check_stmt) == SQLITE_ROW);
    sqlite3_finalize(check_stmt);
    
    if (!chat_exists) {
        std::cerr << "ERROR in addUserToChat: Chat " << chat_id << " not found!" << std::endl;
        return false;
    }
    
    // Основной запрос на добавление
    const char* sql = "INSERT OR IGNORE INTO chat_members (user_id, chat_id) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "ERROR in addUserToChat: Failed to prepare statement for user " 
                  << user_id << " chat " << chat_id 
                  << ". Error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, chat_id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    if (!success) {
        std::cerr << "ERROR in addUserToChat: Failed to execute. SQLite error: " 
                  << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "SUCCESS in addUserToChat: Added user " << user_id 
                  << " to chat " << chat_id << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool Database::removeUserFromChat(int user_id, int chat_id) {
    const char* sql = "DELETE FROM chat_members WHERE user_id = ? AND chat_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, chat_id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}
User* Database::getUserBySession(const std::string& session_token) const {
    const char* sql = "SELECT user_id, username, password_hash, email, session_token FROM users WHERE session_token = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, session_token.c_str(), -1, SQLITE_STATIC);
    
    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Получаем значения из базы данных
        int user_id = sqlite3_column_int(stmt, 0);
        const unsigned char* username_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* password_hash_ptr = sqlite3_column_text(stmt, 2);
        const unsigned char* email_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* session_token_ptr = sqlite3_column_text(stmt, 4);
        
        // Преобразуем в std::string
        std::string username_str = username_ptr ? reinterpret_cast<const char*>(username_ptr) : "";
        std::string password_hash_str = password_hash_ptr ? reinterpret_cast<const char*>(password_hash_ptr) : "";
        std::string email_str = email_ptr ? reinterpret_cast<const char*>(email_ptr) : "";
        std::string session_token_str = session_token_ptr ? reinterpret_cast<const char*>(session_token_ptr) : "";
        
        // Создаем пользователя с помощью конструктора для БД
        user = new User(user_id, username_str, password_hash_str, email_str, session_token_str);
    }
    
    sqlite3_finalize(stmt);
    return user;
}

bool Database::isUserInChat(int user_id, int chat_id) const {
    const char* sql = "SELECT 1 FROM chat_members WHERE user_id = ? AND chat_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, chat_id);
    
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return exists;
}