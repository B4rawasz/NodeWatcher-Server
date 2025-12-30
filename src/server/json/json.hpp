#ifndef JSON_H
#define JSON_H

#include <nlohmann/json.hpp>
#include <variant>

namespace message {
    enum class Type {
        UNKNOWN = -1,
        ERROR = 0,
        AUTH_CHALLENGE = 1,
        AUTH_RESPONSE = 2,
        AUTH_RESULT = 3,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(Type,
                                 {
                                     {Type::UNKNOWN, "UNKNOWN"},
                                     {Type::ERROR, "ERROR"},
                                     {Type::AUTH_CHALLENGE, "AUTH_CHALLENGE"},
                                     {Type::AUTH_RESPONSE, "AUTH_RESPONSE"},
                                     {Type::AUTH_RESULT, "AUTH_RESULT"},
                                 })

    struct Message {
        Type type;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Message, type)

    struct Error : public Message {
        int code;
        std::string message;
        Error() = default;
        Error(int code, const std::string& message)
            : Message(Type::ERROR), code(code), message(message) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Error, type, code, message);

    struct AuthChallenge : public Message {
        std::string nonce;
        AuthChallenge() = default;
        AuthChallenge(const std::string& nonce)
            : Message(Type::AUTH_CHALLENGE), nonce(nonce) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AuthChallenge, type, nonce);

    struct AuthResponse : public Message {
        std::string hmac;
        AuthResponse() = default;
        AuthResponse(const std::string& hmac)
            : Message(Type::AUTH_RESPONSE), hmac(hmac) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AuthResponse, type, hmac);

    struct AuthResult : public Message {
        bool success;
        std::string reason;
        AuthResult() = default;
        AuthResult(bool success, const std::string& reason)
            : Message(Type::AUTH_RESULT), success(success), reason(reason) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AuthResult, type, success, reason);

    using MessageVariant = std::variant<Error, AuthChallenge, AuthResponse, AuthResult>;

    using ParserFn = message::MessageVariant (*)(const nlohmann::json&);

    inline const std::unordered_map<message::Type, ParserFn> parsers = {
        {message::Type::AUTH_CHALLENGE,
         [](const nlohmann::json& j) -> MessageVariant {
             return j.get<message::AuthChallenge>();
         }},
        {message::Type::AUTH_RESPONSE,
         [](const nlohmann::json& j) -> MessageVariant {
             return j.get<message::AuthResponse>();
         }},
        {message::Type::AUTH_RESULT,
         [](const nlohmann::json& j) -> MessageVariant {
             return j.get<message::AuthResult>();
         }},
    };

    inline std::optional<message::MessageVariant> parseMessage(std::string_view payload) {
        try {
            auto j = nlohmann::json::parse(payload);

            message::Message base = j.get<message::Message>();

            auto it = parsers.find(base.type);
            if (it == parsers.end())
                return message::Error{400, "Unknown message type"};

            return it->second(j);
        } catch (const nlohmann::json::exception& e) {
            return message::Error{400, e.what()};
        }
    }

    inline std::string serializeMessage(const message::MessageVariant& msg) {
        return std::visit([](const auto& m) { return nlohmann::json(m).dump(); }, msg);
    }
}  // namespace message

#endif  // JSON_H