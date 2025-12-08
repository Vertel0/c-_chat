#pragma once

#include "../../Crow/include/crow.h"
#include "chat_manager.h"

class WebChatServer {
private:
    crow::SimpleApp app;
    ChatManager chat_manager;
    
public:
    WebChatServer();
    void run(int port = 8080);
    
private:
    void setupRoutes();
    
    // API handlers
    crow::response registerUser(const crow::request& req);
    crow::response loginUser(const crow::request& req);
    crow::response getUserChats(const crow::request& req);
    crow::response getChatMessages(const crow::request& req, int chat_id);
    crow::response sendMessage(const crow::request& req);
    crow::response createChat(const crow::request& req);
    crow::response searchChat(const crow::request& req);
    crow::response joinChat(const crow::request& req);
    
    // Utility
    bool validateRequest(const crow::request& req, int* user_id = nullptr);
};