#include <server.h>
#include "auth.h"
#include "json.hpp"

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::Error& msg) {
    sendJson(ws, message::Error{400, "Unknown message type"});
}

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::AuthResponse& msg) {
    PerSocketData* psd = ws->getUserData();

    std::string expectedHmac = auth::hmacSha256("supersecretkey", psd->nonce);

    if (msg.hmac == expectedHmac) {
        psd->authenticated = true;
        sendJson(ws, message::AuthResult{true, "Authentication successful"});
    } else {
        sendJson(ws, message::AuthResult{false, "Authentication failed"});
        ws->close();
    }
}
