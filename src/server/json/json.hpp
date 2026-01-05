#ifndef JSON_H
#define JSON_H

#include <nlohmann/json.hpp>
#include <string>
#include <variant>

// Message definitions
namespace message {
    enum class Type {
        UNKNOWN = -1,
        ERROR = 0,
        AUTH_CHALLENGE = 1,
        AUTH_RESPONSE = 2,
        AUTH_RESULT = 3,
        SYSTEM_INFO_STATIC = 4,
        SYSTEM_INFO = 5,
        CPU_INFO_STATIC = 6,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(Type,
                                 {
                                     {Type::UNKNOWN, "UNKNOWN"},
                                     {Type::ERROR, "ERROR"},
                                     {Type::AUTH_CHALLENGE, "AUTH_CHALLENGE"},
                                     {Type::AUTH_RESPONSE, "AUTH_RESPONSE"},
                                     {Type::AUTH_RESULT, "AUTH_RESULT"},
                                     {Type::SYSTEM_INFO_STATIC, "SYSTEM_INFO_STATIC"},
                                     {Type::SYSTEM_INFO, "SYSTEM_INFO"},
                                     {Type::CPU_INFO_STATIC, "CPU_INFO_STATIC"},
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

    struct SystemInfoStatic : public Message {
        std::string hostname;
        std::string system_name;
        std::string version_id;
        std::string kernel_version;
        std::string timezone;
        SystemInfoStatic() = default;
        SystemInfoStatic(const std::string& hostname,
                         const std::string& system_name,
                         const std::string& version_id,
                         const std::string& kernel_version,
                         const std::string& timezone)
            : Message(Type::SYSTEM_INFO_STATIC),
              hostname(hostname),
              system_name(system_name),
              version_id(version_id),
              kernel_version(kernel_version),
              timezone(timezone) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SystemInfoStatic,
                                       type,
                                       hostname,
                                       system_name,
                                       version_id,
                                       kernel_version,
                                       timezone);

    struct SystemInfo : public Message {
        std::string uptime;
        std::string local_time;
        SystemInfo() = default;
        SystemInfo(const std::string& uptime, const std::string& local_time)
            : Message(Type::SYSTEM_INFO), uptime(uptime), local_time(local_time) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SystemInfo, type, uptime, local_time);

    struct CpuInfoStatic : public Message {
        std::string cpu_model;
        std::string cpu_architecture;
        int cpu_max_frequency;
        int cpu_cores;
        int cpu_threads;
        CpuInfoStatic() = default;
        CpuInfoStatic(const std::string& cpu_model,
                      const std::string& cpu_architecture,
                      int cpu_max_frequency,
                      int cpu_cores,
                      int cpu_threads)
            : Message(Type::CPU_INFO_STATIC),
              cpu_model(cpu_model),
              cpu_architecture(cpu_architecture),
              cpu_max_frequency(cpu_max_frequency),
              cpu_cores(cpu_cores),
              cpu_threads(cpu_threads) {}
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CpuInfoStatic,
                                       type,
                                       cpu_model,
                                       cpu_architecture,
                                       cpu_max_frequency,
                                       cpu_cores,
                                       cpu_threads);

    struct CpuInfo : public Message {};
}  // namespace message

// Utility functions for parsing and serializing messages
namespace message {

    using MessageVariantIN = std::variant<Error, AuthResponse>;
    using MessageVariantOUT = std::variant<Error,
                                           AuthChallenge,
                                           AuthResult,
                                           SystemInfoStatic,
                                           SystemInfo,
                                           CpuInfoStatic>;

    using MessageVariant = std::variant<Error,
                                        AuthChallenge,
                                        AuthResponse,
                                        AuthResult,
                                        SystemInfoStatic,
                                        SystemInfo,
                                        CpuInfoStatic>;

    using ParserFn = message::MessageVariantIN (*)(const nlohmann::json&);

    inline const std::unordered_map<message::Type, ParserFn> parsers = {
        {message::Type::AUTH_RESPONSE,
         [](const nlohmann::json& j) -> MessageVariantIN {
             return j.get<message::AuthResponse>();
         }},
    };

    inline std::optional<message::MessageVariantIN> parseMessage(
        std::string_view payload) {
        try {
            auto j = nlohmann::json::parse(payload);

            message::Message base = j.get<message::Message>();

            auto it = parsers.find(base.type);
            if (it == parsers.end())
                return message::Error{400, "Unknown message type"};

            return it->second(j);
        } catch (const nlohmann::json::exception& e) {
            return message::Error{e.id, e.what()};
        }
    }

    inline std::string serializeMessage(const message::MessageVariantOUT& msg) {
        return std::visit([](const auto& m) { return nlohmann::json(m).dump(); }, msg);
    }

    inline message::Type getMessageType(const std::string_view payload) {
        try {
            auto j = nlohmann::json::parse(payload);
            message::Message base = j.get<message::Message>();
            return base.type;
        } catch (const nlohmann::json::exception& e) {
            return message::Type::UNKNOWN;
        }
    }
}  // namespace message

#endif  // JSON_H