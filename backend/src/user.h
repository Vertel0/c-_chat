#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <sstream>
#include <chrono>

class User {
private:
    static std::atomic<int> next_id;
    
public:
    int user_id;
    std::string username;
    std::string password_hash; // В реальном проекте нужно хэшировать!
    std::string email;
    std::string session_token;
    std::vector<int> available_chats;
    std::string status;
    std::chrono::system_clock::time_point session_expiry;

    User(const std::string& name, const std::string& password, const std::string& user_email = "");
    
    // Copy constructor for database returns
    User(const User& other);
    
    bool validatePassword(const std::string& password) const;
    std::string generateSessionToken();
    void addChat(int chat_id);
    void removeChat(int chat_id);
    bool hasAccessToChat(int chat_id) const;
    
    // Session management
    bool isSessionValid() const;
    void updateSession();
    
    // Database constructor (for loading from DB)
    User(int id, const std::string& name, const std::string& password, const std::string& user_email, const std::string& token);
};