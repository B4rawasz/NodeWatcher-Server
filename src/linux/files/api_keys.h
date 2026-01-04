#ifndef API_KEYS_H
#define API_KEYS_H

#include <nlohmann/json.hpp>
#include <optional>
#include <shared_mutex>

struct ApiKey {
    std::string key;
    std::string owner;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ApiKey, key, owner);

class KeyStore {
public:
    KeyStore();
    std::optional<ApiKey> getKey(const std::string& owner);
    bool ownerExists(const std::string& owner);
    void addKey(const ApiKey& apiKey);
    void removeKey(const std::string& owner);
    void reload();
    std::string generateNewKey();

private:
    std::shared_mutex mutex;
    std::unordered_map<std::string, ApiKey> keys;
    void ensureKeysDirExists();
    void createEmptyKeysFile();
    void loadKeysJson();
    void saveKeysJson();
};

#endif  // API_KEYS_H