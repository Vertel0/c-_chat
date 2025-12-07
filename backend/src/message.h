#pragma once
#include <string>
#include <chrono>

class Message {
public:
    int message_id;
    int chat_id;
    int sender_id;
    std::string sender_name;
    std::string content;
    std::string timestamp;

    Message(int msg_id, int c_id, int s_id, const std::string& s_name, const std::string& msg);
    
    static std::string getCurrentTimestamp();
};