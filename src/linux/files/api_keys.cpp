#include <api_keys.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <paths.hpp>
#include <random>
#include <shared_mutex>

namespace fs = std::filesystem;

KeyStore::KeyStore() {
    loadKeysJson();
}

std::optional<ApiKey> KeyStore::getKey(const std::string& owner) {
    // Acquire shared lock for reading (multiple readers allowed)
    std::shared_lock lk(mutex);
    auto it = keys.find(owner);
    if (it != keys.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool KeyStore::ownerExists(const std::string& owner) {
    // Acquire shared lock for reading (multiple readers allowed)
    std::shared_lock lk(mutex);
    return keys.find(owner) != keys.end();
}

void KeyStore::addKey(const ApiKey& apiKey) {
    // Acquire unique lock for writing (exclusive access)
    std::unique_lock lk(mutex);
    keys[apiKey.owner] = apiKey;
    saveKeysJson();
}

void KeyStore::removeKey(const std::string& owner) {
    // Acquire unique lock for writing (exclusive access)
    std::unique_lock lk(mutex);
    keys.erase(owner);
    saveKeysJson();
}

void KeyStore::reload() {
    // Acquire unique lock for writing (exclusive access)
    std::unique_lock lk(mutex);
    loadKeysJson();
}

std::string KeyStore::generateNewKey() {
    std::random_device rd;
    std::array<unsigned char, 32> bytes;

    for (auto& byte : bytes) {
        byte = static_cast<unsigned char>(rd());
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        oss << std::setw(2) << static_cast<int>(byte);
    }

    return oss.str();
}

void KeyStore::ensureKeysDirExists() {
    try {
        if (!fs::exists(paths::libDir())) {
            fs::create_directories(paths::libDir());
            chmod(paths::libDir(), 0700);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to create keys directory: ") +
                                 e.what());
    }
}

void KeyStore::createEmptyKeysFile() {
    try {
        std::ofstream out(paths::keysFile(), std::ios::out | std::ios::trunc);
        if (!out)
            throw std::runtime_error("Cannot create keys.json");

        out << "{}\n";
        out.close();

        chmod(paths::keysFile(), 0600);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to create keys.json: ") + e.what());
    }
}

void KeyStore::loadKeysJson() {
    try {
        ensureKeysDirExists();

        if (!fs::exists(paths::keysFile())) {
            createEmptyKeysFile();
        }

        std::ifstream in(paths::keysFile());
        if (!in)
            throw std::runtime_error("Cannot open keys.json");

        nlohmann::json j;
        in >> j;

        if (!j.is_object())
            throw std::runtime_error("keys.json must be a JSON object");

        keys.clear();
        keys = j.get<std::unordered_map<std::string, ApiKey>>();

    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("JSON parse error in keys.json: ") +
                                 e.what());
    } catch (const std::exception& e) {
        throw;
    }
}

void KeyStore::saveKeysJson() {
    nlohmann::json j = keys;
    std::filesystem::path tmp =
        std::filesystem::path(paths::keysFile()).string() + ".tmp";

    std::ofstream out(tmp);
    if (!out)
        throw std::runtime_error("tmp open failed");

    out << j.dump(4);
    out.close();

    std::filesystem::rename(tmp, paths::keysFile());
}