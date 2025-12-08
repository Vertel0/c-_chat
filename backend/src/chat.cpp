#include "chat.h"
#include <algorithm>
#include <sstream>

std::atomic<int> Chat::next_id{1};

Chat::Chat(const std::string& name, int creator_id, const std::string& type, bool public_chat)
    : chat_name(name), chat_type(type), created_by(creator_id), is_public(public_chat) {
    chat_id = next_id++;
    addMember(creator_id); // Создатель автоматически участник
    if (!public_chat) {
        addToWhitelist(creator_id); // И в белом списке
    }
}

void Chat::addToWhitelist(int user_id) {
    if (!isInWhitelist(user_id)) {
        whitelist_ids.push_back(user_id);
    }
}

bool Chat::isInWhitelist(int user_id) const {
    return std::find(whitelist_ids.begin(), whitelist_ids.end(), user_id) != whitelist_ids.end();
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
    // Ограничиваем историю сообщений (опционально)
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

std::string Chat::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"chat_id\":" << chat_id << ","
       << "\"chat_name\":\"" << chat_name << "\","
       << "\"chat_type\":\"" << chat_type << "\","
       << "\"member_count\":" << member_ids.size()
       << "}";
    return ss.str();
}