#include <server.h>
#include <thread>
#include "App.h"

Server::Server(uWS::SocketContextOptions sslOptions) : sslOptions_(sslOptions) {}

Server::~Server() {
    stop();
}

void Server::run() {
    if (running_.load())
        return;

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
    struct PerSocketData {};

    app_ = new uWS::SSLApp(sslOptions_);

    app_->ws<PerSocketData>(
            "/*", {.compression = uWS::DISABLED,
                   .maxPayloadLength = 16 * 1024 * 1024,
                   .idleTimeout = 0,
                   .maxBackpressure = 16 * 1024 * 1024,
                   .closeOnBackpressureLimit = false,
                   .resetIdleTimeoutOnSend = true,
                   .sendPingsAutomatically = false,
                   .upgrade = nullptr,
                   .open =
                       [](auto* ws) {
                           ws->send("Welcome to the WebSocket server!");
                           ws->subscribe("broadcast");
                       },
                   .message = [this](auto* ws, std::string_view message,
                                     uWS::OpCode opCode) { ws->send(message, opCode); },
                   .drain = [](auto*) {},
                   .ping = [](auto*, std::string_view) {},
                   .pong = [](auto*, std::string_view) {},
                   .close = [](auto*, int, std::string_view) {}})
        .listen(9001, [](auto* listen_socket) {
            if (listen_socket) {
                std::cout << "Thread " << std::this_thread::get_id()
                          << " listening on port " << 9001 << std::endl;
            } else {
                std::cout << "Thread " << std::this_thread::get_id()
                          << " failed to listen on port " << 9001 << std::endl;
            }
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