#pragma once
#include <vector>
#include "user.h"
#include "chat.h"
#include "message.h"
#include "database.h"

class ChatManager {
private:
    mutable Database database;

public:
    ChatManager(const std::string& db_path = "chat.db");
    
    // User management
    int registerUser(const std::string& username, const std::string& password, const std::string& email = "");
    User* loginUser(const std::string& username, const std::string& password);
    
    // Chat management
    int createChat(const std::string& chat_name, int creator_id);
    Chat* getChatById(int chat_id);
    std::vector<Chat> getUserChats(int user_id);
    
    // Message management
    bool sendMessage(int chat_id, int sender_id, const std::string& content);
    std::vector<Message> getChatMessages(int chat_id, int user_id, int count = 50);
};