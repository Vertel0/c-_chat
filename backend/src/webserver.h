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
    
    crow::response registerUser(const crow::request& req);
    crow::response loginUser(const crow::request& req);
    crow::response getUserChats(const crow::request& req);
    crow::response getChatMessages(const crow::request& req, int chat_id);
    crow::response sendMessage(const crow::request& req);
    crow::response createChat(const crow::request& req);
    crow::response createChatWithPrivacy(const crow::request& req);
    crow::response addUserToChat(const crow::request& req, int chat_id);
    crow::response searchChat(const crow::request& req);
    crow::response joinChat(const crow::request& req);
    crow::response inviteUserToChat(const crow::request& req, int chat_id);
    
    std::string getSessionToken(const crow::request& req) const;
    bool validateRequest(const crow::request& req, User** user = nullptr);
};