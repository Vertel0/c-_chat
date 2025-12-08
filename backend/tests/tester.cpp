#include "../src/database.h"
#include "../src/user.h"
#include "../src/chat.h"
#include "../src/message.h"
#include "../src/chat_manager.h"
#include <iostream>
#include <cassert>
#include <string>

class ChatTester {
private:
    Database* db;
    ChatManager* chatManager;
    std::string test_db_path;
    int test_version;
    int passed_tests;
    int failed_tests;
    
public:
    ChatTester(int version) : test_version(version), passed_tests(0), failed_tests(0) {
        test_db_path = (version == 2) ? "test_chat_v2.db" : "test_chat_v3.db";
        
        std::remove(test_db_path.c_str());
        
        db = new Database(test_db_path);
        bool initialized = db->initialize();
        if (!initialized) {
            std::cerr << "Failed to initialize database" << std::endl;
            exit(1);
        }
        
        chatManager = new ChatManager(test_db_path);
        
        std::cout << "========================================\n";
        if (test_version == 2) {
            std::cout << "   WEB CHAT v2.0 TESTER\n";
            std::cout << "   (with Search & Join)\n";
        } else {
            std::cout << "   WEB CHAT v3.0 TESTER\n";
            std::cout << "   (with Private Chats & Invites)\n";
        }
        std::cout << "========================================\n";
    }
    
    ~ChatTester() {
        delete chatManager;
        delete db;
        std::remove(test_db_path.c_str());
    }
    
    int runAllTests() {
        std::cout << "\nRunning all tests for Version " << test_version << ".0...\n";
        
        runTest("User Registration", [this]() { testUserRegistration(); });
        runTest("User Login", [this]() { testUserLogin(); });
        
        if (test_version == 2) {
            runTest("Chat Creation", [this]() { testChatCreation(); });
        } else {
            runTest("Chat Creation with Privacy", [this]() { testChatCreationWithPrivacy(); });
        }
        
        runTest("Message Sending", [this]() { testMessageSending(); });
        runTest("User Chats List", [this]() { testUserChats(); });
        runTest("Search Functionality", [this]() { testSearchFunctionality(); });
        
        if (test_version == 2) {
            runTest("Join Functionality", [this]() { testJoinFunctionality(); });
        } else {
            runTest("Public Chat Join", [this]() { testPublicChatJoin(); });
            runTest("Private Chat Access Control", [this]() { testPrivateChatAccess(); });
            runTest("Invite Functionality", [this]() { testInviteFunctionality(); });
        }
        
        runTest("Database Persistence", [this]() { testDatabasePersistence(); });
        
        std::cout << "\n========================================\n";
        std::cout << "   Test summary for v" << test_version << ".0:\n";
        std::cout << "   Passed: " << passed_tests << "\n";
        std::cout << "   Failed: " << failed_tests << "\n";
        
        if (failed_tests == 0) {
            std::cout << "   ALL TESTS PASSED SUCCESSFULLY!\n";
            std::cout << "========================================\n";
            return 0;
        } else {
            std::cout << "   SOME TESTS FAILED!\n";
            std::cout << "========================================\n";
            return 1;
        }
    }
    
private:
    template<typename Func>
    void runTest(const std::string& testName, Func testFunc) {
        std::cout << "\n[TEST] " << testName << "\n";
        std::cout << std::string(testName.length() + 8, '-') << "\n";
        
        try {
            testFunc();
            passed_tests++;
            std::cout << "PASSED\n";
        } catch (const std::exception& e) {
            failed_tests++;
            std::cout << "FAILED: " << e.what() << "\n";
        } catch (...) {
            failed_tests++;
            std::cout << "FAILED: Unknown error\n";
        }
    }
    
    void testUserRegistration() {
        // Тест 1.1: Регистрация нового пользователя
        int user1_id = chatManager->registerUser("alice", "password123", "alice@example.com");
        if (user1_id <= 0) throw std::runtime_error("User registration failed");
        std::cout << "Alice registered with ID: " << user1_id << std::endl;
        
        // Тест 1.2: Регистрация второго пользователя
        int user2_id = chatManager->registerUser("bob", "password456", "bob@example.com");
        if (user2_id <= 0) throw std::runtime_error("Second user registration failed");
        std::cout << "Bob registered with ID: " << user2_id << std::endl;
        
        // Тест 1.3: Регистрация третьего пользователя
        int user3_id = chatManager->registerUser("charlie", "password789", "charlie@example.com");
        if (user3_id <= 0) throw std::runtime_error("Third user registration failed");
        std::cout << "Charlie registered with ID: " << user3_id << std::endl;
        
        if (test_version == 3) {
            // Тест 1.4: Регистрация четвертого пользователя (только для v3)
            int user4_id = chatManager->registerUser("david", "password101", "david@example.com");
            if (user4_id <= 0) throw std::runtime_error("Fourth user registration failed");
            std::cout << "David registered with ID: " << user4_id << std::endl;
        }
    }
    
    void testUserLogin() {
        // Тест 2.1: Успешный логин
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Valid login should succeed");
        
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Should get user by session");
        std::cout << "Alice login successful (ID: " << alice->user_id << ")\n";
        delete alice;
        
        // Тест 2.2: Неправильный пароль
        std::string wrong_token = chatManager->loginUser("alice", "wrongpassword");
        if (!wrong_token.empty()) throw std::runtime_error("Wrong password should fail");
        std::cout << "Wrong password correctly rejected\n";
    }
    
    void testChatCreation() {
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Alice login failed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Could not get Alice");
        
        // Тест 3.1: Создание чата Alice
        int chat1_id = chatManager->createChat("Alice's Public Chat", alice->user_id);
        if (chat1_id <= 0) throw std::runtime_error("Chat creation failed");
        std::cout << "Chat 1 created with ID: " << chat1_id << std::endl;
        
        // Тест 3.2: Создание второго чата Alice
        int chat2_id = chatManager->createChat("Alice's Second Chat", alice->user_id);
        if (chat2_id <= 0) throw std::runtime_error("Second chat creation failed");
        std::cout << "Chat 2 created with ID: " << chat2_id << std::endl;
        
        std::string bob_token = chatManager->loginUser("bob", "password456");
        if (bob_token.empty()) throw std::runtime_error("Bob login failed");
        User* bob = chatManager->getUserBySession(bob_token);
        if (bob == nullptr) throw std::runtime_error("Could not get Bob");
        
        // Тест 3.3: Создание чата Bob
        int chat3_id = chatManager->createChat("Bob's Public Chat", bob->user_id);
        if (chat3_id <= 0) throw std::runtime_error("Bob's chat creation failed");
        std::cout << "Chat 3 created with ID: " << chat3_id << std::endl;
        
        delete alice;
        delete bob;
    }
    
    void testChatCreationWithPrivacy() {
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Alice login failed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Could not get Alice");
        
        std::string bob_token = chatManager->loginUser("bob", "password456");
        if (bob_token.empty()) throw std::runtime_error("Bob login failed");
        User* bob = chatManager->getUserBySession(bob_token);
        if (bob == nullptr) throw std::runtime_error("Could not get Bob");
        
        // Тест 3.1: Создание публичного чата Alice
        int public_chat_id = chatManager->createChat("Alice's Public Chat", alice->user_id, "group", true);
        if (public_chat_id <= 0) throw std::runtime_error("Public chat creation failed");
        std::cout << "Public chat created with ID: " << public_chat_id << std::endl;
        
        // Тест 3.2: Создание приватного чата Alice
        int private_chat_id = chatManager->createChat("Alice's Secret Chat", alice->user_id, "group", false);
        if (private_chat_id <= 0) throw std::runtime_error("Private chat creation failed");
        std::cout << "Private chat created with ID: " << private_chat_id << std::endl;
        
        // Тест 3.3: Создание публичного чата Bob
        int bob_public_chat_id = chatManager->createChat("Bob's Public Lounge", bob->user_id, "group", true);
        if (bob_public_chat_id <= 0) throw std::runtime_error("Bob's public chat creation failed");
        std::cout << "Bob's public chat created with ID: " << bob_public_chat_id << std::endl;
        
        // Тест 3.4: Создание приватного чата Bob
        int bob_private_chat_id = chatManager->createChat("Bob's Private Group", bob->user_id, "group", false);
        if (bob_private_chat_id <= 0) throw std::runtime_error("Bob's private chat creation failed");
        std::cout << "Bob's private chat created with ID: " << bob_private_chat_id << std::endl;
        
        delete alice;
        delete bob;
    }
    
    void testMessageSending() {
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Alice login failed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Could not get Alice");
        
        std::string bob_token = chatManager->loginUser("bob", "password456");
        if (bob_token.empty()) throw std::runtime_error("Bob login failed");
        User* bob = chatManager->getUserBySession(bob_token);
        if (bob == nullptr) throw std::runtime_error("Could not get Bob");
        
        if (test_version == 2) {
            // Alice отправляет сообщение в свой чат
            bool message_sent = chatManager->sendMessage(1, alice->user_id, "Hello from Alice!");
            if (!message_sent) throw std::runtime_error("Message sending failed");
            std::cout << "Alice sent message to her chat\n";
            
            // Bob отправляет сообщение в свой чат
            bool bob_message_sent = chatManager->sendMessage(3, bob->user_id, "Hello from Bob!");
            if (!bob_message_sent) throw std::runtime_error("Bob's message sending failed");
            std::cout << "Bob sent message to his chat\n";
        } else {
            // Alice отправляет сообщение в свой публичный чат
            bool message_sent = chatManager->sendMessage(1, alice->user_id, "Hello from Alice in public chat!");
            if (!message_sent) throw std::runtime_error("Message sending failed in public chat");
            std::cout << "Alice sent message to her public chat\n";
            
            // Alice отправляет сообщение в свой приватный чат
            bool private_message_sent = chatManager->sendMessage(2, alice->user_id, "Hello from Alice in private chat!");
            if (!private_message_sent) throw std::runtime_error("Message sending failed in private chat");
            std::cout << "Alice sent message to her private chat\n";
            
            // Bob отправляет сообщение в свой публичный чат
            bool bob_message_sent = chatManager->sendMessage(3, bob->user_id, "Hello from Bob in public chat!");
            if (!bob_message_sent) throw std::runtime_error("Bob's message sending failed");
            std::cout << "Bob sent message to his public chat\n";
        }
        
        delete alice;
        delete bob;
    }
    
    void testUserChats() {
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Alice login failed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Could not get Alice");
        
        std::string bob_token = chatManager->loginUser("bob", "password456");
        if (bob_token.empty()) throw std::runtime_error("Bob login failed");
        User* bob = chatManager->getUserBySession(bob_token);
        if (bob == nullptr) throw std::runtime_error("Could not get Bob");
        
        if (test_version == 2) {
            // Тест 5.1: Получение списка чатов Alice
            auto alice_chats = chatManager->getUserChats(alice->user_id);
            if (alice_chats.size() != 2) throw std::runtime_error("Alice should have 2 chats");
            std::cout << "Alice has " << alice_chats.size() << " chats\n";
            
            // Тест 5.2: Получение списка чатов Bob
            auto bob_chats = chatManager->getUserChats(bob->user_id);
            if (bob_chats.size() != 1) throw std::runtime_error("Bob should have 1 chat");
            std::cout << "Bob has " << bob_chats.size() << " chat\n";
            
            // Тест 5.3: Проверка изоляции
            bool alice_sees_bobs_chat = false;
            for (const auto& chat : alice_chats) {
                if (chat.chat_name == "Bob's Public Chat") {
                    alice_sees_bobs_chat = true;
                }
            }
            if (alice_sees_bobs_chat) throw std::runtime_error("Alice should not see Bob's chat before joining");
            std::cout << "Chat isolation works (users see only their chats)\n";
        } else {
            // Тест 5.1: Получение списка чатов Alice
            auto alice_chats = chatManager->getUserChats(alice->user_id);
            if (alice_chats.size() != 2) throw std::runtime_error("Alice should have 2 chats (1 public + 1 private)");
            std::cout << "Alice has " << alice_chats.size() << " chats\n";
            
            // Тест 5.2: Получение списка чатов Bob
            auto bob_chats = chatManager->getUserChats(bob->user_id);
            if (bob_chats.size() != 2) throw std::runtime_error("Bob should have 2 chats (1 public + 1 private)");
            std::cout << "Bob has " << bob_chats.size() << " chats\n";
            
            // Проверяем, что Alice не видит приватный чат Bob
            bool alice_sees_bob_private = false;
            for (const auto& chat : alice_chats) {
                if (chat.chat_name == "Bob's Private Group") {
                    alice_sees_bob_private = true;
                }
            }
            if (alice_sees_bob_private) throw std::runtime_error("Alice should not see Bob's private chat");
            std::cout << "Alice cannot see Bob's private chat (correct)\n";
        }
        
        delete alice;
        delete bob;
    }
    
    void testSearchFunctionality() {
        int search_id = (test_version == 2) ? 3 : 1;
        
        // Тест 6.1: Поиск существующего чата
        Chat* found_chat = chatManager->searchChatById(search_id);
        if (found_chat == nullptr) throw std::runtime_error("Should find existing chat");
        
        if (test_version == 2) {
            if (found_chat->chat_name != "Bob's Public Chat") 
                throw std::runtime_error("Wrong chat found");
        } else {
            if (found_chat->chat_name != "Alice's Public Chat") 
                throw std::runtime_error("Wrong chat found");
            if (found_chat->is_public != true) 
                throw std::runtime_error("Should be public chat");
        }
        
        std::cout << "Found chat: " << found_chat->chat_name << " (ID: " << found_chat->chat_id << ")\n";
        
        if (test_version == 3) {
            // Тест 6.2: Поиск приватного чата (только для v3)
            Chat* private_chat = chatManager->searchChatById(2);
            if (private_chat == nullptr) throw std::runtime_error("Should find private chat");
            if (private_chat->chat_name != "Alice's Secret Chat") 
                throw std::runtime_error("Wrong private chat found");
            if (private_chat->is_public != false) 
                throw std::runtime_error("Should be private chat");
            std::cout << "Found private chat: " << private_chat->chat_name << " (ID: " << private_chat->chat_id << ")\n";
            delete private_chat;
        }
        
        // Тест 6.3: Поиск несуществующего чата
        Chat* not_found = chatManager->searchChatById(999);
        if (not_found != nullptr) throw std::runtime_error("Should not find non-existent chat");
        std::cout << "Non-existent chat correctly not found\n";
        
        delete found_chat;
    }
    
    void testJoinFunctionality() {
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Alice login failed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Could not get Alice");
        
        std::string charlie_token = chatManager->loginUser("charlie", "password789");
        if (charlie_token.empty()) throw std::runtime_error("Charlie login failed");
        User* charlie = chatManager->getUserBySession(charlie_token);
        if (charlie == nullptr) throw std::runtime_error("Could not get Charlie");
        
        // Тест 7.1: Alice пытается присоединиться к своему же чату
        bool join_own_chat = chatManager->addUserToChat(alice->user_id, 1);
        if (join_own_chat) throw std::runtime_error("Should not be able to join own chat (already member)");
        std::cout << "Cannot join chat where already a member\n";
        
        // Тест 7.2: Alice присоединяется к чату Bob
        bool join_success = chatManager->addUserToChat(alice->user_id, 3);
        if (!join_success) throw std::runtime_error("Should be able to join Bob's chat");
        std::cout << "Alice successfully joined Bob's chat\n";
        
        // Тест 7.3: Charlie присоединяется к чату Alice
        bool charlie_join = chatManager->addUserToChat(charlie->user_id, 1);
        if (!charlie_join) throw std::runtime_error("Charlie should join Alice's chat");
        std::cout << "Charlie successfully joined Alice's chat\n";
        
        // Тест 7.4: Проверка, что Alice теперь видит чат Bob
        auto alice_chats_after = chatManager->getUserChats(alice->user_id);
        bool alice_sees_bobs_chat = false;
        for (const auto& chat : alice_chats_after) {
            if (chat.chat_id == 3) {
                alice_sees_bobs_chat = true;
            }
        }
        if (!alice_sees_bobs_chat) throw std::runtime_error("Alice should now see Bob's chat after joining");
        std::cout << "Alice now sees Bob's chat in her list\n";
        
        // Тест 7.5: Charlie отправляет сообщение в чат Alice
        bool charlie_message = chatManager->sendMessage(1, charlie->user_id, "Hi from Charlie!");
        if (!charlie_message) throw std::runtime_error("Charlie should send message after joining");
        std::cout << "Charlie can send message after joining\n";
        
        delete alice;
        delete charlie;
    }
    
    void testPublicChatJoin() {
        std::string charlie_token = chatManager->loginUser("charlie", "password789");
        if (charlie_token.empty()) throw std::runtime_error("Charlie login failed");
        User* charlie = chatManager->getUserBySession(charlie_token);
        if (charlie == nullptr) throw std::runtime_error("Could not get Charlie");
        
        std::string david_token = chatManager->loginUser("david", "password101");
        if (david_token.empty()) throw std::runtime_error("David login failed");
        User* david = chatManager->getUserBySession(david_token);
        if (david == nullptr) throw std::runtime_error("Could not get David");
        
        // Тест 7.1: Charlie присоединяется к публичному чату Alice
        bool join_public_success = chatManager->addUserToChat(charlie->user_id, 1);
        if (!join_public_success) throw std::runtime_error("Should be able to join public chat");
        std::cout << "Charlie successfully joined Alice's public chat\n";
        
        // Тест 7.2: David присоединяется к публичному чату Bob
        bool david_join_public = chatManager->addUserToChat(david->user_id, 3);
        if (!david_join_public) throw std::runtime_error("David should join Bob's public chat");
        std::cout << "David successfully joined Bob's public chat\n";
        
        // Тест 7.3: Проверка, что Charlie теперь видит публичный чат Alice
        auto charlie_chats = chatManager->getUserChats(charlie->user_id);
        bool charlie_sees_alice_public = false;
        for (const auto& chat : charlie_chats) {
            if (chat.chat_id == 1) {
                charlie_sees_alice_public = true;
            }
        }
        if (!charlie_sees_alice_public) throw std::runtime_error("Charlie should now see Alice's public chat");
        std::cout << "Charlie now sees Alice's public chat in his list\n";
        
        // Тест 7.4: Charlie отправляет сообщение в публичный чат Alice
        bool charlie_message = chatManager->sendMessage(1, charlie->user_id, "Hi from Charlie in public chat!");
        if (!charlie_message) throw std::runtime_error("Charlie should send message after joining public chat");
        std::cout << "Charlie can send message in public chat after joining\n";
        
        delete charlie;
        delete david;
    }
    
    void testPrivateChatAccess() {
        std::string charlie_token = chatManager->loginUser("charlie", "password789");
        if (charlie_token.empty()) throw std::runtime_error("Charlie login failed");
        User* charlie = chatManager->getUserBySession(charlie_token);
        if (charlie == nullptr) throw std::runtime_error("Could not get Charlie");
        
        // Тест 8.1: Попытка присоединиться к приватному чату без приглашения
        bool join_private_fail = chatManager->addUserToChat(charlie->user_id, 2);
        if (join_private_fail) throw std::runtime_error("Should not be able to join private chat without invitation");
        std::cout << "Charlie correctly rejected from private chat without invitation\n";
        
        // Тест 8.2: Попытка отправить сообщение в приватный чат без доступа
        bool send_to_private_fail = chatManager->sendMessage(2, charlie->user_id, "Should not work!");
        if (send_to_private_fail) throw std::runtime_error("Should not send to private chat without access");
        std::cout << "Charlie cannot send message to private chat without access\n";
        
        // Тест 8.3: Проверка, что Charlie не видит приватный чат в списке
        auto charlie_chats = chatManager->getUserChats(charlie->user_id);
        bool charlie_sees_alice_private = false;
        for (const auto& chat : charlie_chats) {
            if (chat.chat_id == 2) {
                charlie_sees_alice_private = true;
            }
        }
        if (charlie_sees_alice_private) throw std::runtime_error("Charlie should not see private chat in list");
        std::cout << "Charlie does not see private chat in his list\n";
        
        delete charlie;
    }
    
    void testInviteFunctionality() {
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("Alice login failed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("Could not get Alice");
        
        std::string charlie_token = chatManager->loginUser("charlie", "password789");
        if (charlie_token.empty()) throw std::runtime_error("Charlie login failed");
        User* charlie = chatManager->getUserBySession(charlie_token);
        if (charlie == nullptr) throw std::runtime_error("Could not get Charlie");
        
        std::string david_token = chatManager->loginUser("david", "password101");
        if (david_token.empty()) throw std::runtime_error("David login failed");
        User* david = chatManager->getUserBySession(david_token);
        if (david == nullptr) throw std::runtime_error("Could not get David");
        
        // Тест 9.1: Alice приглашает Charlie в свой приватный чат
        bool invite_success = chatManager->addToWhitelist(2, charlie->user_id, alice->user_id);
        if (!invite_success) throw std::runtime_error("Invite should succeed");
        std::cout << "Alice invited Charlie to her private chat\n";
        
        // Тест 9.2: Проверка, что Charlie теперь в whitelist
        bool in_whitelist = chatManager->isUserInWhitelist(charlie->user_id, 2);
        if (!in_whitelist) throw std::runtime_error("Charlie should be in whitelist");
        std::cout << "Charlie is now in whitelist\n";
        
        // Тест 9.3: Теперь Charlie может присоединиться к приватному чату
        bool join_after_invite = chatManager->addUserToChat(charlie->user_id, 2);
        if (!join_after_invite) throw std::runtime_error("Charlie should join after invitation");
        std::cout << "Charlie successfully joined private chat after invitation\n";
        
        // Тест 9.4: Charlie отправляет сообщение в приватный чат
        bool charlie_private_message = chatManager->sendMessage(2, charlie->user_id, "Hi from Charlie in private chat!");
        if (!charlie_private_message) throw std::runtime_error("Charlie should send message in private chat");
        std::cout << "Charlie can send message in private chat after joining\n";
        
        // Тест 9.5: David не может присоединиться к приватному чату (нет приглашения)
        bool david_join_fail = chatManager->addUserToChat(david->user_id, 2);
        if (david_join_fail) throw std::runtime_error("David should not join private chat without invitation");
        std::cout << "David correctly rejected from private chat (no invitation)\n";
        
        // Тест 9.6: Проверка, что Charlie теперь видит приватный чат в списке
        auto charlie_chats = chatManager->getUserChats(charlie->user_id);
        bool charlie_sees_private = false;
        for (const auto& chat : charlie_chats) {
            if (chat.chat_id == 2) {
                charlie_sees_private = true;
            }
        }
        if (!charlie_sees_private) throw std::runtime_error("Charlie should now see private chat in list");
        std::cout << "Charlie now sees private chat in his list\n";
        
        delete alice;
        delete charlie;
        delete david;
    }
    
    void testDatabasePersistence() {
        delete chatManager;
        delete db;
        
        chatManager = new ChatManager(test_db_path);
        
        // Проверяем, что пользователи сохранились
        std::string alice_token = chatManager->loginUser("alice", "password123");
        if (alice_token.empty()) throw std::runtime_error("User login should succeed");
        User* alice = chatManager->getUserBySession(alice_token);
        if (alice == nullptr) throw std::runtime_error("User should persist");
        std::cout << "Alice persisted in database (ID: " << alice->user_id << ")\n";
        
        // Проверяем чаты Alice
        auto alice_chats = chatManager->getUserChats(alice->user_id);
        
        if (test_version == 2) {
            if (alice_chats.size() < 2) throw std::runtime_error("Alice should have at least 2 chats");
        } else {
            if (alice_chats.size() != 2) throw std::runtime_error("Alice should have 2 chats");
        }
        
        std::cout << "Alice's chats persisted (" << alice_chats.size() << " chats)\n";
        
        if (test_version == 3) {
            // Проверяем приватный чат
            Chat* private_chat = chatManager->getChatById(2);
            if (private_chat == nullptr) throw std::runtime_error("Private chat should persist");
            if (private_chat->is_public != false) throw std::runtime_error("Should remain private");
            std::cout << "Private chat persisted with correct privacy setting\n";
            
            // Проверяем whitelist
            std::string charlie_token = chatManager->loginUser("charlie", "password789");
            if (charlie_token.empty()) throw std::runtime_error("Charlie login failed");
            User* charlie = chatManager->getUserBySession(charlie_token);
            if (charlie == nullptr) throw std::runtime_error("Could not get Charlie");
            
            bool whitelist_persisted = chatManager->isUserInWhitelist(charlie->user_id, 2);
            if (!whitelist_persisted) throw std::runtime_error("Whitelist should persist");
            std::cout << "Whitelist persisted correctly\n";
            
            delete charlie;
            delete private_chat;
        }
        
        // Проверяем сообщения
        auto messages = chatManager->getChatMessages(1, alice->user_id, 10);
        
        if (test_version == 2) {
            if (messages.size() < 2) throw std::runtime_error("Should have at least 2 messages");
        } else {
            if (messages.size() < 2) throw std::runtime_error("Should have at least 2 messages in public chat");
            
            auto private_messages = chatManager->getChatMessages(2, alice->user_id, 10);
            if (private_messages.size() < 2) throw std::runtime_error("Should have at least 2 messages in private chat");
            std::cout << "Messages persisted (" << messages.size() + private_messages.size() << " total)\n";
        }
        
        delete alice;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <version>\n";
        std::cerr << "Version: 2 or 3\n";
        return 1;
    }
    
    int version = std::atoi(argv[1]);
    if (version != 2 && version != 3) {
        std::cerr << "Invalid version. Use 2 or 3\n";
        return 1;
    }
    
    std::cout << "Starting Web Chat Version " << version << ".0 Tests\n";
    if (version == 2) {
        std::cout << "Features tested: Search + Join functionality\n";
    } else {
        std::cout << "Features tested: Private Chats + Invite functionality\n";
    }
    std::cout << "========================================\n";
    
    try {
        ChatTester tester(version);
        return tester.runAllTests();
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nTEST FAILED: Unknown error" << std::endl;
        return 1;
    }
}