#include "user.h"
#include <algorithm>
#include <iostream>

std::atomic<int> User::next_id{1};

User::User(const std::string& name, const std::string& password, const std::string& user_email)
    : username(name), password_hash(password), email(user_email) {
    user_id = next_id++;
}

// Copy constructor
User::User(const User& other)
    : user_id(other.user_id),
      username(other.username),
      password_hash(other.password_hash),
      email(other.email),
      available_chats(other.available_chats) {
}

// Database constructor (for loading from DB)
User::User(int id, const std::string& name, const std::string& password, const std::string& user_email)
    : user_id(id), username(name), password_hash(password), email(user_email) {
}

bool User::validatePassword(const std::string& password) const {
    return password_hash == password;
}

void User::addChat(int chat_id) {
    for (int id : available_chats) {
        if (id == chat_id) {
            std::cout << "Chat " << chat_id << " already in user " << user_id << "'s list" << std::endl;
            return;
        }
    }
    
    available_chats.push_back(chat_id);
    std::cout << "Added chat " << chat_id << " to user " << user_id << "'s available chats." << std::endl;
}

void User::removeChat(int chat_id) {
    available_chats.erase(
        std::remove(available_chats.begin(), available_chats.end(), chat_id),
        available_chats.end()
    );
}

bool User::hasAccessToChat(int chat_id) const {
    return std::find(available_chats.begin(), available_chats.end(), chat_id) != available_chats.end();
}