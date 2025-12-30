#include <server.h>
#include <print>

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::Error& msg) {
    PerSocketData* psd = ws->getUserData();
    std::print("Handling Error for client\n");
}

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::AuthChallenge& msg) {
    PerSocketData* psd = ws->getUserData();
    std::print("Handling AuthChallenge for client\n");
}

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::AuthResponse& msg) {
    PerSocketData* psd = ws->getUserData();
    std::print("Handling AuthResponse for client\n");
}

void Server::handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                    const message::AuthResult& msg) {
    PerSocketData* psd = ws->getUserData();
    std::print("Handling AuthResult for client\n");
}