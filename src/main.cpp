#include <App.h>
#include <server.h>
#include <print>
#include <string>

int main(int argc, char** argv) {
    uWS::SocketContextOptions sslOptions = {
        .key_file_name = "../../test/ssl/certs/server.key",
        .cert_file_name = "../../test/ssl/certs/server.crt",
    };

    Server server(sslOptions);
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