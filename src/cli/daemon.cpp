#include <App.h>
#include <api_keys.h>
#include <daemon.h>
#include <server.h>
#include <sys/stat.h>
#include <atomic>
#include <csignal>
#include <fstream>
#include <paths.hpp>

std::atomic<bool> running{true};       // Main loop control
std::atomic<bool> reload_keys{false};  // Flag to trigger keys reload

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        // Graceful shutdown
        running = false;
    } else if (signal == SIGUSR1) {
        // Trigger keys reload
        reload_keys = true;
    }
}

void daemon() {
    // Write PID to file
    std::ofstream pidfile(paths::pidFile());
    if (!std::filesystem::exists(paths::libDir())) {
        try {
            std::filesystem::create_directories(paths::libDir());
            chmod(paths::libDir(), 0700);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to create PID directory: ") +
                                     e.what());
        }
    }

    if (pidfile.is_open()) {
        pidfile << getpid();
        pidfile.close();
    }

    KeyStore keystore;  // Load API keys

    // Initialize server with SSL options
    uWS::SocketContextOptions sslOptions = {
        // DEV only, use real certs in production
        .key_file_name = "../../test/ssl/certs/server.key",
        .cert_file_name = "../../test/ssl/certs/server.crt",
    };

    Server server(sslOptions, keystore);
    server.run(9001);

    while (running) {
        // Sleep for a short duration to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Check if we need to reload keys
        if (reload_keys.exchange(false)) {
            keystore.reload();
        }
    }

    server.stop();

    // Remove PID file
    std::remove(paths::pidFile());
}