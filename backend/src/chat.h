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
    std::string chat_type;
    std::vector<int> member_ids;
    std::vector<int> whitelist_ids;
    std::vector<Message> messages;
    int created_by;
    bool is_public;

    Chat(const std::string& name, int creator_id, const std::string& type = "group", bool public_chat = true);
    
    void addMember(int user_id);
    void addToWhitelist(int user_id);
    bool isInWhitelist(int user_id) const;
    void removeMember(int user_id);
    bool hasMember(int user_id) const;
    void addMessage(const Message& message);
    std::vector<Message> getRecentMessages(int count = 50) const;
    std::string toJson() const;
};