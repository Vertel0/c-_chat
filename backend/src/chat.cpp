#include "chat.h"
#include <algorithm>
#include <sstream>

std::atomic<int> Chat::next_id{1};

Chat::Chat(const std::string& name, int creator_id)
    : chat_name(name), created_by(creator_id) {
    chat_id = next_id++;
    addMember(creator_id);
}

void Chat::addMember(int user_id) {
    if (!hasMember(user_id)) {
        member_ids.push_back(user_id);
    }
}

void Chat::removeMember(int user_id) {
    member_ids.erase(
        std::remove(member_ids.begin(), member_ids.end(), user_id),
        member_ids.end()
    );
}

bool Chat::hasMember(int user_id) const {
    return std::find(member_ids.begin(), member_ids.end(), user_id) != member_ids.end();
}

void Chat::addMessage(const Message& message) {
    messages.push_back(message);
    if (messages.size() > 1000) {
        messages.erase(messages.begin());
    }
}

std::vector<Message> Chat::getRecentMessages(int count) const {
    if (messages.size() <= count) {
        return messages;
    }
    return std::vector<Message>(messages.end() - count, messages.end());
}