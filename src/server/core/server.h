#ifndef server_h
#define server_h
#include <App.h>
#include <api_keys.h>
#include <uuid/uuid.h>
#include <condition_variable>
#include <queue>
#include "json.hpp"

struct PerSocketData {
    uuid_t uuid;
    bool authenticated = false;
    std::string nonce;
    std::chrono::steady_clock::time_point nonceTs;
};

class Server {
public:
    Server(uWS::SocketContextOptions sslOptions, KeyStore& keystore);

    ~Server();

    void run(int port);
    void stop();
    void broadcast(std::string msg);

private:
    void start();
    void flushQueue();

    void onOpen(uWS::WebSocket<true, true, PerSocketData>* ws);
    void onMessage(uWS::WebSocket<true, true, PerSocketData>* ws,
                   std::string_view message,
                   uWS::OpCode opCode);
    void onClose(uWS::WebSocket<true, true, PerSocketData>* ws,
                 int code,
                 std::string_view message);

    void dispatch(uWS::WebSocket<true, true, PerSocketData>* ws,
                  const message::MessageVariantIN& msg);

    void sendJson(uWS::WebSocket<true, true, PerSocketData>* ws,
                  const message::MessageVariantOUT& msg);

    void sendFatalFailure(uWS::WebSocket<true, true, PerSocketData>* ws,
                          const message::MessageVariantOUT& error);

    void handle(uWS::WebSocket<true, true, PerSocketData>* ws, const message::Error& msg);

    void handle(uWS::WebSocket<true, true, PerSocketData>* ws,
                const message::AuthResponse& msg);

    std::thread* wsThread_ = nullptr;
    uWS::SSLApp* app_ = nullptr;
    std::atomic<uWS::Loop*> loop_{nullptr};
    std::mutex loopMutex_;
    std::condition_variable loopCv_;

    std::mutex queueMutex_;
    std::queue<std::string> sendQueue_;
    std::atomic_bool deferScheduled_{false};

    std::atomic<bool> running_{false};

    uWS::SocketContextOptions sslOptions_;
    int port_ = 5055;
    std::string host_ = "";

    KeyStore& keystore_;
};

#endif  // server_h