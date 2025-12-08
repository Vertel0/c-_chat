#include "chat_manager.h"
#include <algorithm>
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
    
    // Create new user (in real project, hash the password!)
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

std::string ChatManager::loginUser(const std::string& username, const std::string& password) {
    User* user = database.getUserByUsername(username);
    if (user && user->validatePassword(password)) {
        std::string session_token = user->generateSessionToken();
        
        // Update session in database
        database.updateUserSession(user->user_id, session_token);
        
        // Update local session map
        std::unique_lock lock(sessions_mutex);
        session_to_user[session_token] = user->user_id;
        
        std::cout << "User logged in: " << username << " (Session: " << session_token << ")" << std::endl;
        
        delete user;
        return session_token;
    }
    
    if (user) delete user;
    return ""; // Invalid credentials
}

bool ChatManager::validateSession(const std::string& session_token) const {
    std::shared_lock lock(sessions_mutex);
    
    // Check local session map first
    if (session_to_user.find(session_token) != session_to_user.end()) {
        return true;
    }
    
    // If not in local map, check database
    User* user = database.getUserBySession(session_token);
    if (user) {
        // Check if session is still valid
        bool valid = user->isSessionValid();
        delete user;
        return valid;
    }
    
    return false;
}

User* ChatManager::getUserBySession(const std::string& session_token) {
    std::shared_lock lock(sessions_mutex);
    
    // Check local session map first
    auto it = session_to_user.find(session_token);
    if (it != session_to_user.end()) {
        return getUserById(it->second);
    }
    
    // Fallback to database lookup
    User* user = database.getUserBySession(session_token);
    if (user && user->isSessionValid()) {
        // Add to local session map for faster access
        std::unique_lock unique_lock(sessions_mutex);
        session_to_user[session_token] = user->user_id;
        return user;
    }
    
    if (user) {
        delete user;
    }
    return nullptr;
}

User* ChatManager::getUserById(int user_id) {
    return database.getUserById(user_id);
}

// Chat management
int ChatManager::createChat(const std::string& chat_name, int creator_id, const std::string& type, bool is_public) {
    int chat_id = database.createChat(chat_name, creator_id, type, is_public);
    
    if (chat_id != -1) {
        std::cout << "Created " << (is_public ? "public" : "private") 
                  << " chat: " << chat_name << " (ID: " << chat_id 
                  << ") by user " << creator_id << std::endl;
    }
    
    return chat_id;
}

bool ChatManager::addUserToChat(int user_id, int chat_id) {
    std::cout << "DEBUG ChatManager::addUserToChat: user " << user_id 
              << " to chat " << chat_id << std::endl;
    
    // Проверяем существование пользователя
    User* user = getUserById(user_id);
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
              << ", Public: " << chat->is_public 
              << ", Members: " << chat->member_ids.size() << std::endl;
    
    // Проверяем доступ для приватных чатов
    if (!chat->is_public && !chat->isInWhitelist(user_id)) {
        std::cout << "ERROR: User " << user_id << " is not in whitelist for private chat " << chat_id << std::endl;
        delete chat;
        return false;
    }
    
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

bool ChatManager::addToWhitelist(int chat_id, int user_id, int invited_by) {
    return database.addToWhitelist(chat_id, user_id, invited_by);
}

bool ChatManager::isUserInWhitelist(int user_id, int chat_id) {
    return database.isUserInWhitelist(user_id, chat_id);
}

bool ChatManager::removeUserFromChat(int user_id, int chat_id) {
    return database.removeUserFromChat(user_id, chat_id);
}

Chat* ChatManager::getChatById(int chat_id) {
    return database.getChatById(chat_id);
}

std::vector<Chat> ChatManager::getUserChats(int user_id) {
    return database.getUserChats(user_id);
}

std::vector<Chat> ChatManager::getAllChats() {
    return database.getAllChats();
}

// Message management
bool ChatManager::sendMessage(int chat_id, int sender_id, const std::string& content, const std::string& type) {
    // Check if user has access to chat
    if (!database.isUserInChat(sender_id, chat_id)) {
        std::cout << "User " << sender_id << " doesn't have access to chat " << chat_id << std::endl;
        return false;
    }
    
    bool success = database.addMessage(chat_id, sender_id, content, type);
    
    if (success) {
        User* user = getUserById(sender_id);
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

// Utility
std::vector<User> ChatManager::getAllUsers() {
    // This would need to be implemented in Database class
    // For now, return empty vector
    return {};
}

void ChatManager::cleanupExpiredSessions() {
    // In real project, clean up expired sessions from database
}