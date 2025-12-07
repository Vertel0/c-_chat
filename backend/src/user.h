#pragma once
#include <string>
#include <vector>
#include <atomic>

class User {
private:
    static std::atomic<int> next_id;
    
public:
    int user_id;
    std::string username;
    std::string password_hash;
    std::string email;
    std::vector<int> available_chats;

    User(const std::string& name, const std::string& password, const std::string& user_email = "");
    
    // Copy constructor
    User(const User& other);
    
    // Database constructor (for loading from DB)
    User(int id, const std::string& name, const std::string& password, const std::string& user_email);
    
    bool validatePassword(const std::string& password) const;
    void addChat(int chat_id);
    void removeChat(int chat_id);
    bool hasAccessToChat(int chat_id) const;
};