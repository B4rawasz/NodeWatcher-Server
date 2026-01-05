#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <light_module.h>
#include <chrono>
#include <stop_token>
#include <thread>
#include <unordered_map>

class Scheduler {
public:
    using Clock = std::chrono::steady_clock;

    Scheduler() = default;
    ~Scheduler() = default;

    void add(ILightModule* m);

    void start();
    void stop();

private:
    void run(std::stop_token st);

    std::vector<ILightModule*> modules_;
    std::unordered_map<ILightModule*, Clock::time_point> nextRun_;

    std::mutex mutex_;
    std::atomic<bool> running_{false};
    std::jthread worker_;
};

#endif  // SCHEDULER_H