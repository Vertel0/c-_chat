#include "webserver.h"
#include <iostream>
#include <sstream>

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

crow::response WebChatServer::joinChat(const crow::request& req) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        int chat_id = json["chat_id"].i();
        std::cout << "DEBUG joinChat: User " << user->user_id 
                  << " (" << user->username << ") attempting to join chat " 
                  << chat_id << std::endl;
        Chat* chat = chat_manager.getChatById(chat_id);
        if (!chat) {
            return crow::response(404, "Chat not found");
        }
        
        if (!chat->is_public && !chat->isInWhitelist(user->user_id)) {
            delete chat;
            return crow::response(403, "This is a private chat. You need an invitation to join.");
        }
        
        if (chat->hasMember(user->user_id)) {
            delete chat;
            return crow::response(409, "User already in this chat");
        }
        
        delete chat;
        
        bool success = chat_manager.addUserToChat(user->user_id, chat_id);
        if (!success) {
            return crow::response(500, "Failed to join chat");
        }
        
        crow::json::wvalue response;
        response["message"] = "Successfully joined chat";
        response["chat_id"] = chat_id;
        
        return crow::response{response};
        
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION in joinChat: " << e.what() << std::endl;
        return crow::response(500, std::string("Server error: ") + e.what());
    }
}


crow::response WebChatServer::searchChat(const crow::request& req) {
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        int chat_id = json["chat_id"].i();
        
        Chat* chat = chat_manager.getChatById(chat_id);
        if (!chat) {
            return crow::response(404, "Chat not found");
        }
        
        crow::json::wvalue response;
        response["chat_id"] = chat->chat_id;
        response["chat_name"] = chat->chat_name;
        response["chat_type"] = chat->chat_type;
        response["member_count"] = chat->member_ids.size();
        response["created_by"] = chat->created_by;
        response["is_public"] = chat->is_public;
        
        delete chat;
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

void WebChatServer::setupRoutes() {
    CROW_ROUTE(app, "/")
    ([this]() {
        std::string html = loadTemplate("templates/index.html");
        return html;
    });
    
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

    CROW_ROUTE(app, "/api/chats/create_with_privacy").methods("POST"_method)
    ([this](const crow::request& req) {
    return createChatWithPrivacy(req);
    });

    CROW_ROUTE(app, "/api/chats/<int>/invite").methods("POST"_method)
    ([this](const crow::request& req, int chat_id) {
    return inviteUserToChat(req, chat_id);
    });
    
    CROW_ROUTE(app, "/api/chats/<int>/add_user").methods("POST"_method)
    ([this](const crow::request& req, int chat_id) {
        return addUserToChat(req, chat_id);
    });

    CROW_ROUTE(app, "/api/chats/search").methods("POST"_method)
    ([this](const crow::request& req) {
        return searchChat(req);
    });

    CROW_ROUTE(app, "/api/chats/join").methods("POST"_method)
    ([this](const crow::request& req) {
        return joinChat(req);
    });
}

crow::response WebChatServer::createChatWithPrivacy(const crow::request& req) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        std::string chat_name = json["chat_name"].s();
        bool is_public = json["is_public"].b();
        
        if (chat_name.empty()) {
            return crow::response(400, "Chat name cannot be empty");
        }
        
        int chat_id = chat_manager.createChat(chat_name, user->user_id, "group", is_public);
        
        if (chat_id == -1) {
            return crow::response(500, "Failed to create chat");
        }
        
        crow::json::wvalue response;
        response["chat_id"] = chat_id;
        response["is_public"] = is_public;
        response["message"] = std::string("Chat created successfully (") + 
                              (is_public ? "public" : "private") + ")";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

crow::response WebChatServer::inviteUserToChat(const crow::request& req, int chat_id) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        int target_user_id = json["user_id"].i();
        
        Chat* chat = chat_manager.getChatById(chat_id);
        if (!chat) {
            return crow::response(404, "Chat not found");
        }
        
        if (chat->is_public) {
            delete chat;
            return crow::response(400, "Cannot invite to public chat");
        }
        
        if (!chat->hasMember(user->user_id)) {
            delete chat;
            return crow::response(403, "You are not a member of this chat");
        }
        
        delete chat;
        
        bool success = chat_manager.addToWhitelist(chat_id, target_user_id, user->user_id);
        if (!success) {
            return crow::response(500, "Failed to invite user");
        }
        
        crow::json::wvalue response;
        response["message"] = "User invited successfully";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}


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
        
        std::string session_token = chat_manager.loginUser(username, password);
        if (session_token.empty()) {
            return crow::response(401, "Invalid credentials");
        }
        
        User* user = chat_manager.getUserBySession(session_token);
        if (!user) {
            return crow::response(500, "Failed to get user info");
        }
        
        crow::json::wvalue response;
        response["session_token"] = session_token;
        response["user_id"] = user->user_id;
        response["message"] = "Login successful";
        
        delete user;
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

crow::response WebChatServer::getUserChats(const crow::request& req) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    auto user_chats = chat_manager.getUserChats(user->user_id);
    crow::json::wvalue response;
    
    response["chats"] = crow::json::wvalue::list();
    
    int i = 0;
    for (const auto& chat : user_chats) {
        response["chats"][i] = crow::json::wvalue();
        response["chats"][i]["chat_id"] = chat.chat_id;
        response["chats"][i]["chat_name"] = chat.chat_name;
        response["chats"][i]["chat_type"] = chat.chat_type;
        response["chats"][i]["member_count"] = chat.member_ids.size();
        i++;
    }
    
    std::cout << "Sending chats response for user " << user->user_id << ": " << response.dump() << std::endl;
    
    return crow::response{response};
}

crow::response WebChatServer::getChatMessages(const crow::request& req, int chat_id) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    auto messages = chat_manager.getChatMessages(chat_id, user->user_id);
    crow::json::wvalue response;
    
    int i = 0;
    for (const auto& msg : messages) {
        response["messages"][i]["message_id"] = msg.message_id;
        response["messages"][i]["sender_id"] = msg.sender_id;
        response["messages"][i]["sender_name"] = msg.sender_name;
        response["messages"][i]["content"] = msg.content;
        response["messages"][i]["timestamp"] = msg.timestamp;
        response["messages"][i]["type"] = msg.message_type;
        i++;
    }
    
    return crow::response{response};
}

crow::response WebChatServer::sendMessage(const crow::request& req) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        int chat_id = json["chat_id"].i();
        std::string content = json["content"].s();
        
        if (content.empty()) {
            return crow::response(400, "Message content cannot be empty");
        }
        
        bool success = chat_manager.sendMessage(chat_id, user->user_id, content);
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
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        std::string chat_name = json["chat_name"].s();
        if (chat_name.empty()) {
            return crow::response(400, "Chat name cannot be empty");
        }
        
        int chat_id = chat_manager.createChat(chat_name, user->user_id);
        
        std::cout << "Created chat: " << chat_name << " (ID: " << chat_id << ") for user " << user->user_id << std::endl;
        
        crow::json::wvalue response;
        response["chat_id"] = chat_id;
        response["message"] = "Chat created successfully";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

crow::response WebChatServer::addUserToChat(const crow::request& req, int chat_id) {
    User* user = nullptr;
    if (!validateRequest(req, &user)) {
        return crow::response(401, "Invalid session");
    }
    
    try {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");
        
        int target_user_id = json["user_id"].i();
        
        bool success = chat_manager.addUserToChat(target_user_id, chat_id);
        if (!success) {
            return crow::response(404, "User or chat not found");
        }
        
        crow::json::wvalue response;
        response["message"] = "User added to chat successfully";
        return crow::response{response};
        
    } catch (const std::exception& e) {
        return crow::response(500, "Server error");
    }
}

std::string WebChatServer::getSessionToken(const crow::request& req) const {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.find("Bearer ") == 0) {
        return auth_header.substr(7);
    }
    return "";
}

bool WebChatServer::validateRequest(const crow::request& req, User** user) {
    std::string session_token = getSessionToken(req);
    if (session_token.empty()) return false;
    
    User* found_user = chat_manager.getUserBySession(session_token);
    if (!found_user) return false;
    
    if (user) *user = found_user;
    return true;
}