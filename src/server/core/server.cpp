#include <server.h>
#include <print>
#include "auth.h"
#include "json.hpp"

Server::Server(uWS::SocketContextOptions sslOptions, KeyStore& keystore)
    : sslOptions_(sslOptions), keystore_(keystore) {}

Server::~Server() {
    stop();
}

void Server::run(int port) {
    if (running_.load())
        return;

    port_ = port;

    wsThread_ = new std::thread(&Server::start, this);

    std::unique_lock lk(loopMutex_);

    loopCv_.wait(lk, [&] { return loop_ != nullptr; });

    running_.store(true);
}

void Server::stop() {
    if (!running_.load())
        return;

    auto* loop = loop_.load();

    if (loop) {
        loop->defer([this] { app_->close(); });
    }

    if (wsThread_ && wsThread_->joinable()) {
        wsThread_->join();
    }

    delete wsThread_;
    wsThread_ = nullptr;
    running_.store(false);
}

void Server::broadcast(std::string msg) {
    {
        std::lock_guard lk(queueMutex_);
        sendQueue_.push(std::move(msg));
    }

    uWS::Loop* loop = loop_.load();

    if (!loop)
        return;

    if (!deferScheduled_.exchange(true)) {
        loop->defer([this]() {
            deferScheduled_ = false;
            flushQueue();
        });
    }
}

void Server::start() {
    app_ = new uWS::SSLApp(sslOptions_);

    app_->ws<PerSocketData>(
            "/*",
            {.compression = uWS::DISABLED,
             .maxPayloadLength = 16 * 1024 * 1024,
             .idleTimeout = 0,
             .maxBackpressure = 16 * 1024 * 1024,
             .closeOnBackpressureLimit = false,
             .resetIdleTimeoutOnSend = true,
             .sendPingsAutomatically = false,
             .upgrade = nullptr,
             .open = [this](auto* ws) { onOpen(ws); },
             .message = [this](auto* ws, std::string_view message,
                               uWS::OpCode opCode) { onMessage(ws, message, opCode); },
             .close = [this](auto* ws, int code,
                             std::string_view message) { onClose(ws, code, message); }})
        .listen(port_, [this](auto* listen_socket) {
            /*if (listen_socket) {
                std::cout << "Thread " << std::this_thread::get_id()
                          << " listening on port " << port_ << std::endl;
            } else {
                std::cout << "Thread " << std::this_thread::get_id()
                          << " failed to listen on port " << port_ << std::endl;
            }*/
        });

    {
        std::lock_guard lk(loopMutex_);
        loop_ = uWS::Loop::get();
    }
    loopCv_.notify_all();

    app_->run();

    delete app_;
    app_ = nullptr;

    uWS::Loop::get()->free();
}

void Server::flushQueue() {
    if (uWS::Loop::get() != loop_.load()) {
        std::abort();
    }

    std::queue<std::string> local;

    {
        std::lock_guard lk(queueMutex_);
        std::swap(local, sendQueue_);
    }

    while (!local.empty()) {
        const std::string& msg = local.front();

        app_->publish("broadcast", msg, uWS::OpCode::TEXT);

        local.pop();
    }
}

void Server::onOpen(uWS::WebSocket<true, true, PerSocketData>* ws) {
    PerSocketData* psd = ws->getUserData();

    uuid_generate(psd->uuid);
    psd->nonce = auth::generateNonce();
    psd->nonceTs = std::chrono::steady_clock::now();

    sendJson(ws, message::AuthChallenge{psd->nonce});
}

void Server::onMessage(uWS::WebSocket<true, true, PerSocketData>* ws,
                       std::string_view message,
                       uWS::OpCode opCode) {
    PerSocketData* psd = ws->getUserData();

    if (!psd->authenticated) {
        if (psd->nonceTs + std::chrono::seconds(5) < std::chrono::steady_clock::now()) {
            sendJson(ws, message::Error{402, "Authentication nonce expired"});
            uWS::Loop::get()->defer([ws]() { ws->close(); });
            return;
        }

        if (message::getMessageType(message) != message::Type::AUTH_RESPONSE) {
            sendJson(ws, message::Error{401, "Authentication required"});
            return;
        }

        dispatch(ws, message::parseMessage(message).value());
        return;
    }

    ws->send(message, opCode);
}

void Server::onClose(uWS::WebSocket<true, true, PerSocketData>* ws,
                     int code,
                     std::string_view message) {
    PerSocketData* psd = ws->getUserData();
    char uuidStr[37];
    uuid_unparse_lower(psd->uuid, uuidStr);
    std::print("Connection with client {} closed. Code: {}, Message: {}\n", uuidStr, code,
               message);
}

void Server::dispatch(uWS::WebSocket<true, true, PerSocketData>* ws,
                      const message::MessageVariantIN& msg) {
    std::visit([&](auto&& m) { handle(ws, m); }, msg);
}

void Server::sendJson(uWS::WebSocket<true, true, PerSocketData>* ws,
                      const message::MessageVariantOUT& msg) {
    std::string t = message::serializeMessage(msg);
    ws->send(message::serializeMessage(msg), uWS::OpCode::TEXT);
}

void Server::sendFatalFailure(uWS::WebSocket<true, true, PerSocketData>* ws,
                              const message::MessageVariantOUT& error) {
    sendJson(ws, error);
    uWS::Loop::get()->defer([ws]() { ws->close(); });
}