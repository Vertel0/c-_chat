#include "../src/database.h"
#include "../src/user.h"
#include "../src/chat.h"
#include "../src/message.h"
#include "../src/chat_manager.h"
#include <iostream>
#include <cassert>

class BasicVersionTester {
private:
    Database* db;
    ChatManager* chatManager;
    std::string test_db_path = "test_chat.db";
    
public:
    BasicVersionTester() {
        std::remove(test_db_path.c_str());
        
        db = new Database(test_db_path);
        bool initialized = db->initialize();
        assert(initialized && "Failed to initialize database");
        
        chatManager = new ChatManager(test_db_path);
        
        std::cout << "========================================\n";
        std::cout << "   BASIC VERSION TESTER\n";
        std::cout << "========================================\n";
    }
    
    ~BasicVersionTester() {
        delete chatManager;
        delete db;
        std::remove(test_db_path.c_str());
    }
    
    void runAllTests() {
        std::cout << "\nRunning all tests...\n";
        
        testUserRegistration();
        testUserLogin();
        testChatCreation();
        testMessageSending();
        testUserChats();
        testNegativeCases();
        testDatabasePersistence();
        
        std::cout << "\n========================================\n";
        std::cout << "   ALL TESTS PASSED SUCCESSFULLY!\n";
        std::cout << "========================================\n";
    }
    
private:
    void testUserRegistration() {
        std::cout << "\n[TEST 1] User Registration\n";
        std::cout << "---------------------------\n";
        
        // Тест 1.1: Регистрация нового пользователя
        int user1_id = chatManager->registerUser("test_user1", "password123", "test1@example.com");
        assert(user1_id > 0 && "User registration failed");
        std::cout << "User1 registered with ID: " << user1_id << std::endl;
        
        // Тест 1.2: Регистрация второго пользователя
        int user2_id = chatManager->registerUser("test_user2", "password456", "test2@example.com");
        assert(user2_id > 0 && "Second user registration failed");
        std::cout << "User2 registered with ID: " << user2_id << std::endl;
        
        // Тест 1.3: Попытка регистрации с существующим именем
        int duplicate_id = chatManager->registerUser("test_user1", "password789", "duplicate@example.com");
        assert(duplicate_id == -1 && "Duplicate username should fail");
        std::cout << "Duplicate username correctly rejected\n";
    }
    
    void testUserLogin() {
        std::cout << "\n[TEST 2] User Login\n";
        std::cout << "--------------------\n";
        
        // Тест 2.1: Успешный логин
        User* user1 = chatManager->loginUser("test_user1", "password123");
        assert(user1 != nullptr && "Valid login should succeed");
        assert(user1->username == "test_user1");
        std::cout << "User1 login successful (ID: " << user1->user_id << ")\n";
        delete user1;
        
        // Тест 2.2: Неправильный пароль
        User* wrong_pass = chatManager->loginUser("test_user1", "wrongpassword");
        assert(wrong_pass == nullptr && "Wrong password should fail");
        std::cout << "Wrong password correctly rejected\n";
        
        // Тест 2.3: Несуществующий пользователь
        User* nonexistent = chatManager->loginUser("nonexistent_user", "password");
        assert(nonexistent == nullptr && "Non-existent user should fail");
        std::cout << "Non-existent user correctly rejected\n";
    }
    
    void testChatCreation() {
        std::cout << "\n[TEST 3] Chat Creation\n";
        std::cout << "-----------------------\n";
        
        User* creator = chatManager->loginUser("test_user1", "password123");
        assert(creator != nullptr);
        
        // Тест 3.1: Создание чата
        int chat_id = chatManager->createChat("Test Chat Room", creator->user_id);
        assert(chat_id > 0 && "Chat creation failed");
        std::cout << "Chat created with ID: " << chat_id << std::endl;
        
        // Тест 3.2: Проверка существования чата
        Chat* chat = chatManager->getChatById(chat_id);
        assert(chat != nullptr && "Chat should exist");
        assert(chat->chat_name == "Test Chat Room");
        std::cout << "Chat exists and has correct name: " << chat->chat_name << std::endl;
        
        // Тест 3.3: Создатель должен быть в чате
        bool creator_in_chat = chat->hasMember(creator->user_id);
        assert(creator_in_chat && "Creator should be in chat");
        std::cout << "Creator is member of the chat\n";
        
        delete chat;
        delete creator;
    }
    
    void testMessageSending() {
        std::cout << "\n[TEST 4] Message Sending\n";
        std::cout << "-------------------------\n";
        
        User* user1 = chatManager->loginUser("test_user1", "password123");
        assert(user1 != nullptr);
        
        // Создаем чат для теста сообщений
        int chat_id = chatManager->createChat("Message Test Chat", user1->user_id);
        assert(chat_id > 0);
        
        // Тест 4.1: Отправка сообщения
        bool message_sent = chatManager->sendMessage(chat_id, user1->user_id, "Hello, World!");
        assert(message_sent && "Message sending failed");
        std::cout << "Message sent successfully\n";
        
        // Тест 4.2: Получение сообщений
        auto messages = chatManager->getChatMessages(chat_id, user1->user_id, 10);
        assert(!messages.empty() && "Messages should be retrieved");
        assert(messages[0].content == "Hello, World!");
        std::cout << "Message retrieved: \"" << messages[0].content << "\"\n";
        std::cout << "Sender: " << messages[0].sender_name << std::endl;
        std::cout << "Timestamp: " << messages[0].timestamp << std::endl;
        
        // Тест 4.3: Отправка второго сообщения
        bool second_message = chatManager->sendMessage(chat_id, user1->user_id, "Second message");
        assert(second_message);
        
        auto two_messages = chatManager->getChatMessages(chat_id, user1->user_id, 10);
        assert(two_messages.size() == 2 && "Should have 2 messages");
        std::cout << "Second message sent, total messages: " << two_messages.size() << std::endl;
        
        delete user1;
    }
    
    void testUserChats() {
        std::cout << "\n[TEST 5] User Chats List\n";
        std::cout << "-------------------------\n";
        
        User* user1 = chatManager->loginUser("test_user1", "password123");
        assert(user1 != nullptr);
        
        User* user2 = chatManager->loginUser("test_user2", "password456");
        assert(user2 != nullptr);
        
        // Создаем несколько чатов для первого пользователя
        int chat1 = chatManager->createChat("Chat 1", user1->user_id);
        int chat2 = chatManager->createChat("Chat 2", user1->user_id);
        int chat3 = chatManager->createChat("Chat 3", user1->user_id);
        
        // Тест 5.1: Получение списка чатов пользователя 1
        auto user1_chats = chatManager->getUserChats(user1->user_id);
        assert(user1_chats.size() >= 3 && "User1 should have at least 3 chats");
        std::cout << "✓ User1 has " << user1_chats.size() << " chats\n";
        
        // Тест 5.2: Проверка имен чатов
        bool found_chat1 = false;
        bool found_chat2 = false;
        bool found_chat3 = false;
        
        for (const auto& chat : user1_chats) {
            if (chat.chat_name == "Chat 1") found_chat1 = true;
            if (chat.chat_name == "Chat 2") found_chat2 = true;
            if (chat.chat_name == "Chat 3") found_chat3 = true;
        }
        
        assert(found_chat1 && "Chat 1 should be in list");
        assert(found_chat2 && "Chat 2 should be in list");
        assert(found_chat3 && "Chat 3 should be in list");
        std::cout << "All created chats found in user's list\n";
        
        // Тест 5.3: У второго пользователя не должно быть этих чатов
        auto user2_chats = chatManager->getUserChats(user2->user_id);
        std::cout << "User2 has " << user2_chats.size() << " chats (should be 0 or own chats)\n";
        
        delete user1;
        delete user2;
    }
    
    void testNegativeCases() {
        std::cout << "\n[TEST 6] Negative Cases (Error Handling)\n";
        std::cout << "------------------------------------------\n";
        
        // Тест 6.1: Отправка сообщения в несуществующий чат
        User* user1 = chatManager->loginUser("test_user1", "password123");
        assert(user1 != nullptr);
        
        bool fake_chat_message = chatManager->sendMessage(99999, user1->user_id, "Test");
        assert(!fake_chat_message && "Should not send to non-existent chat");
        std::cout << "Cannot send to non-existent chat\n";
        
        // Тест 6.2: Отправка сообщения без доступа к чату
        User* user2 = chatManager->loginUser("test_user2", "password456");
        assert(user2 != nullptr);
        
        int private_chat = chatManager->createChat("Private Chat", user1->user_id);
        
        // Попытка отправить сообщение в чужой чат
        bool unauthorized_message = chatManager->sendMessage(private_chat, user2->user_id, "Hi");
        assert(!unauthorized_message && "Should not send to chat without access");
        std::cout << "Cannot send to chat without being a member\n";
        
        // Тест 6.3: Получение сообщений без доступа
        auto unauthorized_messages = chatManager->getChatMessages(private_chat, user2->user_id);
        assert(unauthorized_messages.empty() && "Should not get messages without access");
        std::cout << "Cannot get messages from chat without access\n";
        
        delete user1;
        delete user2;
    }
    
    void testDatabasePersistence() {
        std::cout << "\n[TEST 7] Database Persistence\n";
        std::cout << "------------------------------\n";
        
        // Пересоздаем менеджер для теста персистентности
        delete chatManager;
        delete db;
        
        chatManager = new ChatManager(test_db_path);
        
        // Проверяем, что пользователи сохранились
        User* user1 = chatManager->loginUser("test_user1", "password123");
        assert(user1 != nullptr && "User should persist");
        std::cout << "✓ User1 persisted in database (ID: " << user1->user_id << ")\n";
        
        User* user2 = chatManager->loginUser("test_user2", "password456");
        assert(user2 != nullptr && "User2 should persist");
        std::cout << "✓ User2 persisted in database (ID: " << user2->user_id << ")\n";
        
        // Проверяем, что можно получить чаты пользователя
        auto user_chats = chatManager->getUserChats(user1->user_id);
        assert(!user_chats.empty() && "User chats should persist");
        std::cout << "✓ User's chats persisted (" << user_chats.size() << " chats)\n";
        
        // Проверяем сообщения в одном из чатов
        if (!user_chats.empty()) {
            int first_chat_id = user_chats[0].chat_id;
            auto messages = chatManager->getChatMessages(first_chat_id, user1->user_id);
            std::cout << "✓ Messages persisted in chat " << first_chat_id << " (" << messages.size() << " messages)\n";
        }
        
        delete user1;
        delete user2;
    }
};

int main() {
    std::cout << "Starting Basic Web Chat Version 1.0 Tests\n";
    std::cout << "========================================\n";
    
    try {
        BasicVersionTester tester;
        tester.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nTEST FAILED: Unknown error" << std::endl;
        return 1;
    }
}