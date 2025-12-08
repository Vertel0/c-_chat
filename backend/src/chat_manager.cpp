#include "chat_manager.h"
#include <iostream>

ChatManager::ChatManager(const std::string& db_path) : database(db_path) {
    database.initialize();
}

// User management
int ChatManager::registerUser(const std::string& username, const std::string& password, const std::string& email) {
    // Check if user already exists
    User* existing_user = database.getUserByUsername(username);
    if (existing_user) {
        delete existing_user;
        return -1; // User already exists
    }
    
    // Create new user
    if (database.createUser(username, password, email)) {
        User* new_user = database.getUserByUsername(username);
        if (new_user) {
            int user_id = new_user->user_id;
            delete new_user;
            return user_id;
        }
    }
    
    return -1;
}

User* ChatManager::loginUser(const std::string& username, const std::string& password) {
    User* user = database.getUserByUsername(username);
    if (user && user->validatePassword(password)) {
        return user;
    }
    
    if (user) delete user;
    return nullptr; // Invalid credentials
}

// Chat management
int ChatManager::createChat(const std::string& chat_name, int creator_id) {
    return database.createChat(chat_name, creator_id);
}

bool ChatManager::addUserToChat(int user_id, int chat_id) {
    std::cout << "DEBUG ChatManager::addUserToChat: user " << user_id 
              << " to chat " << chat_id << std::endl;
    
    // Проверяем существование пользователя
    User* user = database.getUserById(user_id);
    if (!user) {
        std::cout << "ERROR: User " << user_id << " not found!" << std::endl;
        return false;
    }
    delete user;
    
    Chat* chat = getChatById(chat_id);
    if (!chat) {
        std::cout << "ERROR: Chat " << chat_id << " not found!" << std::endl;
        return false;
    }
    
    std::cout << "DEBUG: Chat found. Name: " << chat->chat_name 
              << ", Members: " << chat->member_ids.size() << std::endl;
    
    // Проверяем, не состоит ли уже
    if (chat->hasMember(user_id)) {
        std::cout << "ERROR: User " << user_id << " already in chat " << chat_id << std::endl;
        delete chat;
        return false;
    }
    
    bool success = database.addUserToChat(user_id, chat_id);
    
    if (success) {
        std::cout << "SUCCESS: Added user " << user_id << " to chat " << chat_id << std::endl;
    } else {
        std::cout << "ERROR: Database failed to add user " << user_id << " to chat " << chat_id << std::endl;
    }
    
    delete chat;
    return success;
}

Chat* ChatManager::getChatById(int chat_id) {
    return database.getChatById(chat_id);
}

std::vector<Chat> ChatManager::getUserChats(int user_id) {
    return database.getUserChats(user_id);
}

// Message management
bool ChatManager::sendMessage(int chat_id, int sender_id, const std::string& content) {
    // Check if user has access to chat
    if (!database.isUserInChat(sender_id, chat_id)) {
        std::cout << "User " << sender_id << " doesn't have access to chat " << chat_id << std::endl;
        return false;
    }
    
    bool success = database.addMessage(chat_id, sender_id, content);
    
    if (success) {
        User* user = database.getUserById(sender_id);
        if (user) {
            std::cout << "Message from " << user->username << " in chat " << chat_id << ": " << content << std::endl;
            delete user;
        }
    }
    
    return success;
}

std::vector<Message> ChatManager::getChatMessages(int chat_id, int user_id, int count) {
    // Check if user has access to chat
    if (!database.isUserInChat(user_id, chat_id)) {
        return {};
    }
    
    return database.getChatMessages(chat_id, count);
}

// Search functionality
Chat* ChatManager::searchChatById(int chat_id) {
    return database.getChatById(chat_id);
}