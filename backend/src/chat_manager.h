#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include "user.h"
#include "chat.h"
#include "message.h"
#include "database.h"

class ChatManager {
private:
    mutable Database database;
    std::unordered_map<std::string, int> session_to_user; // session_token -> user_id
    
    mutable std::shared_mutex sessions_mutex;
    
    User* getUserById(int user_id);

public:
    ChatManager(const std::string& db_path = "chat.db");
    
    // User management
    int registerUser(const std::string& username, const std::string& password, const std::string& email = "");
    std::string loginUser(const std::string& username, const std::string& password);
    bool validateSession(const std::string& session_token) const;
    User* getUserBySession(const std::string& session_token);
    
    // Chat management
    int createChat(const std::string& chat_name, int creator_id, const std::string& type = "group", bool is_public = true); // ← ИЗМЕНЕНО
    bool addUserToChat(int user_id, int chat_id);
    bool removeUserFromChat(int user_id, int chat_id);
    Chat* getChatById(int chat_id);
    std::vector<Chat> getUserChats(int user_id);
    std::vector<Chat> getAllChats();
    
    // Whitelist management (для приватных чатов) ← ДОБАВЛЕНО
    bool addToWhitelist(int chat_id, int user_id, int invited_by);
    bool isUserInWhitelist(int user_id, int chat_id);
    
    // Message management
    bool sendMessage(int chat_id, int sender_id, const std::string& content, const std::string& type = "text");
    std::vector<Message> getChatMessages(int chat_id, int user_id, int count = 50);
    
    // Search functionality
    Chat* searchChatById(int chat_id);
    
    // Utility
    std::vector<User> getAllUsers();
    void cleanupExpiredSessions();
};