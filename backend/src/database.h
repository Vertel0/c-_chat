#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "user.h"
#include "chat.h"
#include "message.h"

class Database {
private:
    sqlite3* db;
    std::string db_path;
    
public:
    Database(const std::string& path);
    ~Database();
    
    bool initialize();
    
    // User operations
    bool createUser(const std::string& username, const std::string& password_hash, const std::string& email);
    User* getUserByUsername(const std::string& username) const;
    User* getUserById(int user_id) const;
    
    // Chat operations
    int createChat(const std::string& chat_name, int creator_id);
    Chat* getChatById(int chat_id) const;
    std::vector<Chat> getUserChats(int user_id) const;
    
    // Message operations
    bool addMessage(int chat_id, int sender_id, const std::string& content);
    std::vector<Message> getChatMessages(int chat_id, int limit = 50) const;
    
    // Membership operations
    bool addUserToChat(int user_id, int chat_id);
    bool isUserInChat(int user_id, int chat_id) const;
    
private:
    void close();
};