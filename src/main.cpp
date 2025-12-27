#include <App.h>
#include <server.h>
#include <print>
#include <string>

int main(int argc, char** argv) {
    Server server;
    server.run();

    while (true) {
        std::println("Input message to broadcast (or 'exit' to quit):");
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit") {
            break;
        }
        server.broadcast(input);
    }

    server.stop();
    return 0;
}