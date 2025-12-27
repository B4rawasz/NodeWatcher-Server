#ifndef server_h
#define server_h
#include <App.h>
#include <condition_variable>
#include <queue>

class Server {
public:
    Server();
    Server(const char* certFile, const char* keyFile);

    ~Server();

    void run();
    void stop();
    void broadcast(std::string msg);

private:
    void start();
    void flushQueue();

    std::thread* wsThread_ = nullptr;
    uWS::App* app_ = nullptr;
    std::atomic<uWS::Loop*> loop_{nullptr};
    std::mutex loopMutex_;
    std::condition_variable loopCv_;

    std::mutex queueMutex_;
    std::queue<std::string> sendQueue_;
    std::atomic_bool deferScheduled_{false};

    std::atomic<bool> running{false};
};

#endif  // server_h