#include <auth.h>
#include <openssl/hmac.h>
#include <random>

namespace auth {
std::string generateNonce() {
    static const char chars[] = "0123456789abcdef";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(0, 15);

    std::string nonce;
    for (int i = 0; i < 32; i++)
        nonce += chars[d(gen)];

    return nonce;
}

std::string hmacSha256(const std::string& key, const std::string& data) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(EVP_sha256(), key.data(), key.size(), (unsigned char*)data.data(), data.size(),
         result, &len);

    return std::string((char*)result, len);
}
}  // namespace auth