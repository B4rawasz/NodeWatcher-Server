#ifndef server_h
#define server_h
#include <condition_variable>
#include <queue>
#include "App.h"

class Server {
public:
    Server(uWS::SocketContextOptions sslOptions);

    ~Server();

    void run();
    void stop();
    void broadcast(std::string msg);

private:
    void start();
    void flushQueue();

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
};

#endif  // server_h