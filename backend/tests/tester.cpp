#include "../src/database.h"
#include "../src/user.h"
#include "../src/chat.h"
#include "../src/message.h"
#include "../src/chat_manager.h"
#include <iostream>
#include <cassert>

class Version2Tester {
private:
    Database* db;
    ChatManager* chatManager;
    std::string test_db_path = "test_chat_v2.db";
    
public:
    Version2Tester() {
        std::remove(test_db_path.c_str());
        
        db = new Database(test_db_path);
        bool initialized = db->initialize();
        assert(initialized && "Failed to initialize database");
        
        chatManager = new ChatManager(test_db_path);
        
        std::cout << "========================================\n";
        std::cout << "   WEB CHAT v2.0 TESTER\n";
        std::cout << "   (with Search & Join)\n";
        std::cout << "========================================\n";
    }
    
    ~Version2Tester() {
        delete chatManager;
        delete db;
        std::remove(test_db_path.c_str());
    }
    
    void runAllTests() {
        std::cout << "\nRunning all tests for Version 2.0...\n";
        
        testUserRegistration();
        testUserLogin();
        testChatCreation();
        testMessageSending();
        testUserChats();
        testSearchFunctionality();
        testJoinFunctionality();
        testDatabasePersistence();
        
        std::cout << "\n========================================\n";
        std::cout << "   ALL v2.0 TESTS PASSED SUCCESSFULLY!\n";
        std::cout << "========================================\n";
    }
    
private:
    void testUserRegistration() {
        std::cout << "\n[TEST 1] User Registration\n";
        std::cout << "---------------------------\n";
        
        // Тест 1.1: Регистрация нового пользователя
        int user1_id = chatManager->registerUser("alice_v2", "password123", "alice@example.com");
        assert(user1_id > 0 && "User registration failed");
        std::cout << "Alice registered with ID: " << user1_id << std::endl;
        
        // Тест 1.2: Регистрация второго пользователя
        int user2_id = chatManager->registerUser("bob_v2", "password456", "bob@example.com");
        assert(user2_id > 0 && "Second user registration failed");
        std::cout << "Bob registered with ID: " << user2_id << std::endl;
        
        // Тест 1.3: Регистрация третьего пользователя
        int user3_id = chatManager->registerUser("charlie_v2", "password789", "charlie@example.com");
        assert(user3_id > 0 && "Third user registration failed");
        std::cout << "Charlie registered with ID: " << user3_id << std::endl;
    }
    
    void testUserLogin() {
        std::cout << "\n[TEST 2] User Login\n";
        std::cout << "--------------------\n";
        
        // Тест 2.1: Успешный логин
        User* alice = chatManager->loginUser("alice_v2", "password123");
        assert(alice != nullptr && "Valid login should succeed");
        std::cout << "Alice login successful (ID: " << alice->user_id << ")\n";
        delete alice;
        
        // Тест 2.2: Неправильный пароль
        User* wrong_pass = chatManager->loginUser("alice_v2", "wrongpassword");
        assert(wrong_pass == nullptr && "Wrong password should fail");
        std::cout << "Wrong password correctly rejected\n";
    }
    
    void testChatCreation() {
        std::cout << "\n[TEST 3] Chat Creation\n";
        std::cout << "-----------------------\n";
        
        User* alice = chatManager->loginUser("alice_v2", "password123");
        assert(alice != nullptr);
        
        // Тест 3.1: Создание чата Alice
        int chat1_id = chatManager->createChat("Alice's Public Chat", alice->user_id);
        assert(chat1_id > 0 && "Chat creation failed");
        std::cout << "Chat 1 created with ID: " << chat1_id << std::endl;
        
        // Тест 3.2: Создание второго чата Alice
        int chat2_id = chatManager->createChat("Alice's Second Chat", alice->user_id);
        assert(chat2_id > 0);
        std::cout << "Chat 2 created with ID: " << chat2_id << std::endl;
        
        User* bob = chatManager->loginUser("bob_v2", "password456");
        assert(bob != nullptr);
        
        // Тест 3.3: Создание чата Bob
        int chat3_id = chatManager->createChat("Bob's Public Chat", bob->user_id);
        assert(chat3_id > 0);
        std::cout << "Chat 3 created with ID: " << chat3_id << std::endl;
        
        delete alice;
        delete bob;
    }
    
    void testMessageSending() {
        std::cout << "\n[TEST 4] Message Sending\n";
        std::cout << "-------------------------\n";
        
        User* alice = chatManager->loginUser("alice_v2", "password123");
        assert(alice != nullptr);
        
        // Alice отправляет сообщение в свой чат
        bool message_sent = chatManager->sendMessage(1, alice->user_id, "Hello from Alice!");
        assert(message_sent && "Message sending failed");
        std::cout << "Alice sent message to her chat\n";
        
        delete alice;
    }
    
    void testUserChats() {
        std::cout << "\n[TEST 5] User Chats List\n";
        std::cout << "-------------------------\n";
        
        User* alice = chatManager->loginUser("alice_v2", "password123");
        assert(alice != nullptr);
        
        User* bob = chatManager->loginUser("bob_v2", "password456");
        assert(bob != nullptr);
        
        // Тест 5.1: Получение списка чатов Alice
        auto alice_chats = chatManager->getUserChats(alice->user_id);
        assert(alice_chats.size() == 2 && "Alice should have 2 chats");
        std::cout << "Alice has " << alice_chats.size() << " chats\n";
        
        // Тест 5.2: Получение списка чатов Bob
        auto bob_chats = chatManager->getUserChats(bob->user_id);
        assert(bob_chats.size() == 1 && "Bob should have 1 chat");
        std::cout << "Bob has " << bob_chats.size() << " chat\n";
        
        // Тест 5.3: Проверка изоляции
        bool alice_sees_bobs_chat = false;
        for (const auto& chat : alice_chats) {
            if (chat.chat_name == "Bob's Public Chat") {
                alice_sees_bobs_chat = true;
            }
        }
        assert(!alice_sees_bobs_chat && "Alice should not see Bob's chat before joining");
        std::cout << "Chat isolation works (users see only their chats)\n";
        
        delete alice;
        delete bob;
    }
    
    void testSearchFunctionality() {
        std::cout << "\n[TEST 6] Search Functionality\n";
        std::cout << "------------------------------\n";
        
        // Тест 6.1: Поиск существующего чата
        Chat* found_chat = chatManager->searchChatById(3);
        assert(found_chat != nullptr && "Should find existing chat");
        assert(found_chat->chat_name == "Bob's Public Chat");
        std::cout << "Found chat: " << found_chat->chat_name << " (ID: " << found_chat->chat_id << ")\n";
        std::cout << "Members: " << found_chat->member_ids.size() << std::endl;
        
        // Тест 6.2: Поиск несуществующего чата
        Chat* not_found = chatManager->searchChatById(999);
        assert(not_found == nullptr && "Should not find non-existent chat");
        std::cout << "Non-existent chat correctly not found\n";
        
        delete found_chat;
    }
    
    void testJoinFunctionality() {
        std::cout << "\n[TEST 7] Join Functionality\n";
        std::cout << "---------------------------\n";
        
        User* alice = chatManager->loginUser("alice_v2", "password123");
        assert(alice != nullptr);
        
        User* charlie = chatManager->loginUser("charlie_v2", "password789");
        assert(charlie != nullptr);
        
        // Тест 7.1: Alice пытается присоединиться к своему же чату
        bool join_own_chat = chatManager->addUserToChat(alice->user_id, 1);
        assert(!join_own_chat && "Should not be able to join own chat (already member)");
        std::cout << "Cannot join chat where already a member\n";
        
        // Тест 7.2: Alice присоединяется к чату Bob
        bool join_success = chatManager->addUserToChat(alice->user_id, 3);
        assert(join_success && "Should be able to join Bob's chat");
        std::cout << "Alice successfully joined Bob's chat\n";
        
        // Тест 7.3: Charlie присоединяется к чату Alice
        bool charlie_join = chatManager->addUserToChat(charlie->user_id, 1);
        assert(charlie_join && "Charlie should join Alice's chat");
        std::cout << "Charlie successfully joined Alice's chat\n";
        
        // Тест 7.4: Проверка, что Alice теперь видит чат Bob
        auto alice_chats_after = chatManager->getUserChats(alice->user_id);
        bool alice_sees_bobs_chat = false;
        for (const auto& chat : alice_chats_after) {
            if (chat.chat_id == 3) {
                alice_sees_bobs_chat = true;
            }
        }
        assert(alice_sees_bobs_chat && "Alice should now see Bob's chat after joining");
        std::cout << "Alice now sees Bob's chat in her list\n";
        
        // Тест 7.5: Charlie отправляет сообщение в чат Alice
        bool charlie_message = chatManager->sendMessage(1, charlie->user_id, "Hi from Charlie!");
        assert(charlie_message && "Charlie should send message after joining");
        std::cout << "Charlie can send message after joining\n";
        
        delete alice;
        delete charlie;
    }
    
    void testDatabasePersistence() {
        std::cout << "\n[TEST 8] Database Persistence\n";
        std::cout << "------------------------------\n";
        
        delete chatManager;
        delete db;
        
        chatManager = new ChatManager(test_db_path);
        
        // Проверяем, что пользователи сохранились
        User* alice = chatManager->loginUser("alice_v2", "password123");
        assert(alice != nullptr && "User should persist");
        std::cout << "Alice persisted in database (ID: " << alice->user_id << ")\n";
        
        // Проверяем чаты Alice
        auto alice_chats = chatManager->getUserChats(alice->user_id);
        assert(alice_chats.size() >= 3 && "Alice should have at least 3 chats (2 own + 1 joined)");
        std::cout << "Alice's chats persisted (" << alice_chats.size() << " chats)\n";
        
        // Проверяем сообщения
        auto messages = chatManager->getChatMessages(1, alice->user_id);
        assert(messages.size() >= 2 && "Should have at least 2 messages");
        std::cout << "Messages persisted (" << messages.size() << " messages)\n";
        
        delete alice;
    }
};

int main() {
    std::cout << "Starting Web Chat Version 2.0 Tests\n";
    std::cout << "Features tested: Search + Join functionality\n";
    std::cout << "========================================\n";
    
    try {
        Version2Tester tester;
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