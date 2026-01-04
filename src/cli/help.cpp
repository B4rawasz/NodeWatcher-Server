#include <help.h>
#include <print>

void help() {
    std::println(R"(
{}NodeWatcher Server{} - System Monitoring Tool

{}USAGE:{}
    nodewatcher [COMMAND]

{}COMMANDS:{}
    {}--help{}          Display this help message
    {}--keygen{}        Generate a new API key
    {}--version{}       Show version information
)",
                 "\033[1;32m", "\033[0m",  // Green Bold
                 "\033[1m", "\033[0m",     // Bold for usage
                 "\033[1m", "\033[0m",     // Bold for commands
                 "\033[36m", "\033[0m",    // Cyan for commands
                 "\033[36m", "\033[0m",    // Cyan for commands
                 "\033[36m", "\033[0m"     // Cyan for commands
    );
}