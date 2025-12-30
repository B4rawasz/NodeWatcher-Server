#ifndef AUTH_H
#define AUTH_H

#include <string>

namespace auth {
    std::string generateNonce();
    std::string hmacSha256(const std::string& key, const std::string& data);
};  // namespace auth

#endif  // AUTH_H