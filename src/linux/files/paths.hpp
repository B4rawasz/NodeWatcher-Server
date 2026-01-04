#ifndef PATHS_H
#define PATHS_H

#include <linux/limits.h>
#include <unistd.h>
#include <filesystem>
#include <string>

namespace paths {
    inline std::string getExecutablePath() {
        std::filesystem::path exe = std::filesystem::read_symlink("/proc/self/exe");

        return exe.parent_path().string();
    }

    inline const char* libDir() {
        static std::string path;
        if (path.empty()) {
            const char* env = std::getenv("NODEWATCHER_ENV");
            if (env && std::string(env) == "development") {
                path = getExecutablePath() + "/dev/lib";
            } else {
                path = "/var/lib/nodewatcher";
            }
        }
        return path.c_str();
    }

    inline const char* keysFile() {
        static std::string path = std::string(libDir()) + "/keys.json";
        return path.c_str();
    }

    inline const char* pidFile() {
        static std::string path = std::string(libDir()) + "/nodewatcher.pid";
        return path.c_str();
    }
}  // namespace paths

#endif  // PATHS_H