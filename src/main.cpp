#include <daemon.h>
#include <help.h>
#include <keygen.h>
#include <csignal>
#include <print>
#include <string>

int main(int argc, char** argv) {
    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGUSR1, signalHandler);

    std::string env = "production";

    const char* value = std::getenv("NODEWATCHER_ENV");
    if (value != nullptr) {
        env = std::string(value);
    }

    // Auto-daemonize if run by systemd or in development environment
    if (std::getenv("INVOCATION_ID") != nullptr || (env == "development" && argc == 1)) {
        daemon();
        return 0;
    }

    std::string argvStr[argc];
    for (int i = 0; i < argc; ++i) {
        argvStr[i] = argv[i];
    }

    if (argc < 2) {
        help();
        return 1;
    }

    std::string command = argvStr[1];

    if (command == "--keygen") {
        keygen();
    } else if (command == "--help") {
        help();
    } else if (command == "--version") {
        std::println("NodeWatcher Server - Version 1.0.0");
    } else {
        help();
        return 1;
    }

    return 0;
}