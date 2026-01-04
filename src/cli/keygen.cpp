#include <api_keys.h>
#include <keygen.h>
#include <signal.h>
#include <fstream>
#include <iostream>
#include <paths.hpp>
#include <print>

void keygen() {
    std::ifstream pidfile(paths::pidFile());
    if (pidfile.is_open()) {
        pid_t pid;
        pidfile >> pid;
        pidfile.close();

        KeyStore keystore;
        ApiKey newKey;

        while (true) {
            std::println(
                "Enter owner name for the new API key (alphanumeric and hyphens only, "
                "max 64 "
                "characters). Type 'quit' to cancel:");
            std::getline(std::cin, newKey.owner);
            if (newKey.owner.empty()) {
                std::println("Owner name cannot be empty. Please try again.");
            } else if (keystore.ownerExists(newKey.owner)) {
                std::println(
                    "An API key with this owner name already exists. Please choose a "
                    "different name.");
            } else if (newKey.owner.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGH"
                                                      "IJKLMNOPQRSTUVWXYZ0123456789-") !=
                       std::string::npos) {
                std::println(
                    "Owner name contains invalid characters. Only alphanumeric "
                    "characters, and hyphens are allowed. Please try again.");
            } else if (newKey.owner.length() > 64) {
                std::println(
                    "Owner name is too long (maximum 64 characters). Please try again.");
            } else if (newKey.owner == "quit" || newKey.owner == "exit") {
                std::println("Operation cancelled by user.");
                return;
            } else {
                break;
            }
        }

        newKey.key = keystore.generateNewKey();

        keystore.addKey(newKey);
        std::println("New API key generated:");
        std::println("Owner: {}", newKey.owner);
        std::println("Key: {}", newKey.key);

        kill(pid, SIGUSR1);  // Notify daemon to reload keys
    } else {
        std::println(stderr, "Daemon PID file not found. Is the daemon running?");
    }
}