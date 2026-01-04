#include <server.h>
#include <print>
#include "auth.h"
#include "json.hpp"

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::Error& msg) {
    // For variant completeness, no specific handling needed. Users shuldn't send error
    // messages.
    sendJson(ws, message::Error{400, "Unknown message type"});
}

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::AuthResponse& msg) {
    PerSocketData* psd = ws->getUserData();

    // Extract user and HMAC from the message
    // Format assumed: "user_hmac"
    std::string user, hmac;

    user = msg.hmac.find('_') != std::string::npos
               ? msg.hmac.substr(0, msg.hmac.find('_'))
               : "";
    hmac = msg.hmac.find('_') != std::string::npos
               ? msg.hmac.substr(msg.hmac.find('_') + 1)
               : msg.hmac;

    // Lookup API key for the user
    auto it = apiKeys_.find(user);

    if (it == apiKeys_.end()) {
        // User not found
        sendFatalFailure(ws, message::AuthResult{false, "Invalid API key"});
        return;
    }

    apiKeys::ApiKey apiKey = it->second;

    // Compute expected HMAC
    std::string expectedHmac = auth::hmacSha256(apiKey.key, psd->nonce);

    if (hmac == expectedHmac) {
        // Authentication successful
        psd->authenticated = true;
        sendJson(ws, message::AuthResult{true, "Authentication successful"});
    } else {
        // Authentication failed
        sendFatalFailure(ws, message::AuthResult{false, "Authentication failed"});
    }
}
