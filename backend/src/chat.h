#pragma once
#include <string>
#include <vector>
#include <atomic>
#include "message.h"

class Chat {
private:
    static std::atomic<int> next_id;
    
public:
    int chat_id;
    std::string chat_name;
    std::vector<int> member_ids;
    std::vector<Message> messages;
    int created_by;

    Chat(const std::string& name, int creator_id);
    
    void addMember(int user_id);
    void removeMember(int user_id);
    bool hasMember(int user_id) const;
    void addMessage(const Message& message);
    std::vector<Message> getRecentMessages(int count = 50) const;
};