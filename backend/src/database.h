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
    User* getUserBySession(const std::string& session_token) const;
    bool updateUserSession(int user_id, const std::string& session_token);
    
    // Chat operations  
    int createChat(const std::string& chat_name, int creator_id, const std::string& type = "group", bool is_public = true);
    Chat* getChatById(int chat_id) const;
    std::vector<Chat> getUserChats(int user_id) const;
    std::vector<Chat> getAllChats() const;
    
    // Whitelist operations - для приватных чатов
    bool addToWhitelist(int chat_id, int user_id, int invited_by);
    bool isUserInWhitelist(int user_id, int chat_id) const;
    
    // Message operations
    bool addMessage(int chat_id, int sender_id, const std::string& content, const std::string& type = "text");
    std::vector<Message> getChatMessages(int chat_id, int limit = 50) const;
    
    // Membership operations
    bool addUserToChat(int user_id, int chat_id);
    bool removeUserFromChat(int user_id, int chat_id);
    bool isUserInChat(int user_id, int chat_id) const;
    
    // Utility
    std::vector<User> getAllUsers() const;
    
private:
    void close();
};
