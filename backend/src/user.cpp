#include "user.h"
#include <random>
#include <sstream>
#include <algorithm>
#include <iostream>

std::atomic<int> User::next_id{1};

User::User(const std::string& name, const std::string& password, const std::string& user_email)
    : username(name), password_hash(password), // В реальном проекте хэшируйте пароль!
      email(user_email), status("offline") {
    user_id = next_id++;
    session_token = generateSessionToken();
    updateSession(); // Устанавливаем время истечения сессии
}

// Copy constructor
User::User(const User& other)
    : user_id(other.user_id),
      username(other.username),
      password_hash(other.password_hash),
      email(other.email),
      session_token(other.session_token),
      available_chats(other.available_chats),
      status(other.status),
      session_expiry(other.session_expiry) {
}

// Database constructor (for loading from DB)
User::User(int id, const std::string& name, const std::string& password, 
           const std::string& user_email, const std::string& token)
    : user_id(id), username(name), password_hash(password),
      email(user_email), session_token(token), status("offline") {
    updateSession(); // Устанавливаем время истечения сессии
}

bool User::validatePassword(const std::string& password) const {
    return password_hash == password; // Простая проверка - в реальном проекте используйте хэши!
}

std::string User::generateSessionToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

void User::addChat(int chat_id) {
    // Проверяем, нет ли уже этого чата
    for (int id : available_chats) {
        if (id == chat_id) {
            std::cout << "Chat " << chat_id << " already in user " << user_id << "'s list" << std::endl;
            return;
        }
    }
    
    available_chats.push_back(chat_id);
    std::cout << "Added chat " << chat_id << " to user " << user_id << "'s available chats. Now has " << available_chats.size() << " chats." << std::endl;
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

// Session management
bool User::isSessionValid() const {
    return std::chrono::system_clock::now() < session_expiry;
}

void User::updateSession() {
    session_expiry = std::chrono::system_clock::now() + std::chrono::hours(24 * 7); // 1 неделя
}