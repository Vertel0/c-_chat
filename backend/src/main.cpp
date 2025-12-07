#include "webserver.h"
#include <iostream>

int main() {
    try {
        WebChatServer server;
        server.run(8080);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}