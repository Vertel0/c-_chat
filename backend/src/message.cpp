#include "message.h"
#include <sstream>
#include <iomanip>

Message::Message(int msg_id, int c_id, int s_id, const std::string& s_name, const std::string& msg)
    : message_id(msg_id), chat_id(c_id), sender_id(s_id), 
      sender_name(s_name), content(msg) {
    timestamp = getCurrentTimestamp();
}

std::string Message::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}