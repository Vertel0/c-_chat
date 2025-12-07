#include "webserver.h"
#include <iostream>
#include <sstream>
#include <fstream>

WebChatServer::WebChatServer() {
    setupRoutes();
}

void WebChatServer::run(int port) {
    std::cout << "Web Chat Server running on http://localhost:" << port << std::endl;
    app.port(port).multithreaded().run();
}

std::string loadTemplate(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open template file: " << filename << std::endl;
        return "<html><body><h1>Error: Template not found</h1></body></html>";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void WebChatServer::setupRoutes() {
    // Serve main page with direct HTML
    CROW_ROUTE(app, "/")
    ([this]() {
        std::string html = loadTemplate("templates/index.html");
        return html;
    });
    
    // API routes
    CROW_ROUTE(app, "/api/register").methods("POST"_method)
    ([this](const crow::request& req) {
        return registerUser(req);
    });
    
    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([this](const crow::request& req) {
        return loginUser(req);
    });
    
    CROW_ROUTE(app, "/api/chats").methods("GET"_method)
    ([this](const crow::request& req) {
        return getUserChats(req);
    });
    
    CROW_ROUTE(app, "/api/chats/<int>/messages").methods("GET"_method)
    ([this](const crow::request& req, int chat_id) {
        return getChatMessages(req, chat_id);
    });
    
    CROW_ROUTE(app, "/api/messages").methods("POST"_method)
    ([this](const crow::request& req) {
        return sendMessage(req);
    });
    
    CROW_ROUTE(app, "/api/chats/create").methods("POST"_method)
    ([this](const crow::request& req) {
        return createChat(req);
    });
}

// API Handlers implementation
crow::response WebChatServer::registerUser(const crow::request& req) {
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        std::string username = json["username"].s();
        std::string password = json["password"].s();
        std::string email = json["email"].s();
        
        if (username.empty() || password.empty()) {
            return crow::response(400, "Username and password required");
        }
        
        int user_id = chat_manager.registerUser(username, password, email);
        if (user_id == -1) {
            return crow::response(409, "Username already exists");
        }
        
        crow::json::wvalue response;
        response["user_id"] = user_id;
        response["message"] = "User registered successfully";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

crow::response WebChatServer::loginUser(const crow::request& req) {
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        std::string username = json["username"].s();
        std::string password = json["password"].s();
        
        User* user = chat_manager.loginUser(username, password);
        if (!user) {
            return crow::response(401, "Invalid credentials");
        }
        
        crow::json::wvalue response;
        response["user_id"] = user->user_id;
        response["username"] = user->username;
        response["message"] = "Login successful";
        
        delete user;
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

crow::response WebChatServer::getUserChats(const crow::request& req) {
    int user_id = 0;
    if (!validateRequest(req, &user_id)) {
        return crow::response(401, "Invalid request");
    }
    
    auto user_chats = chat_manager.getUserChats(user_id);
    crow::json::wvalue response;
    
    response["chats"] = crow::json::wvalue::list();
    
    int i = 0;
    for (const auto& chat : user_chats) {
        response["chats"][i] = crow::json::wvalue();
        response["chats"][i]["chat_id"] = chat.chat_id;
        response["chats"][i]["chat_name"] = chat.chat_name;
        response["chats"][i]["member_count"] = chat.member_ids.size();
        i++;
    }
    
    return crow::response{response};
}

crow::response WebChatServer::getChatMessages(const crow::request& req, int chat_id) {
    int user_id = 0;
    if (!validateRequest(req, &user_id)) {
        return crow::response(401, "Invalid request");
    }
    
    auto messages = chat_manager.getChatMessages(chat_id, user_id);
    crow::json::wvalue response;
    
    int i = 0;
    for (const auto& msg : messages) {
        response["messages"][i]["message_id"] = msg.message_id;
        response["messages"][i]["sender_id"] = msg.sender_id;
        response["messages"][i]["sender_name"] = msg.sender_name;
        response["messages"][i]["content"] = msg.content;
        response["messages"][i]["timestamp"] = msg.timestamp;
        i++;
    }
    
    return crow::response{response};
}

crow::response WebChatServer::sendMessage(const crow::request& req) {
    int user_id = 0;
    if (!validateRequest(req, &user_id)) {
        return crow::response(401, "Invalid request");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        int chat_id = json["chat_id"].i();
        std::string content = json["content"].s();
        
        if (content.empty()) {
            return crow::response(400, "Message content cannot be empty");
        }
        
        bool success = chat_manager.sendMessage(chat_id, user_id, content);
        if (!success) {
            return crow::response(403, "No access to chat or chat doesn't exist");
        }
        
        crow::json::wvalue response;
        response["message"] = "Message sent successfully";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

crow::response WebChatServer::createChat(const crow::request& req) {
    int user_id = 0;
    if (!validateRequest(req, &user_id)) {
        return crow::response(401, "Invalid request");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        std::string chat_name = json["chat_name"].s();
        if (chat_name.empty()) {
            return crow::response(400, "Chat name cannot be empty");
        }
        
        int chat_id = chat_manager.createChat(chat_name, user_id);
        
        if (chat_id == -1) {
            return crow::response(500, "Failed to create chat");
        }
        
        crow::json::wvalue response;
        response["chat_id"] = chat_id;
        response["message"] = "Chat created successfully";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

// Utility methods
bool WebChatServer::validateRequest(const crow::request& req, int* user_id) {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.find("Basic ") != 0) {
        return false;
    }
    
    // Basic auth: "Basic user_id:password_hash" (упрощенно)
    std::string credentials = auth_header.substr(6);
    size_t colon_pos = credentials.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }
    
    try {
        int id = std::stoi(credentials.substr(0, colon_pos));
        if (user_id) *user_id = id;
        return true;
    } catch (...) {
        return false;
    }
}