#include <scheduler.h>

void Scheduler::add(ILightModule* m) {
    std::lock_guard<std::mutex> lock(mutex_);
    modules_.push_back(m);
    nextRun_[m] = Clock::now();
}

void Scheduler::start() {
    if (running_.exchange(true))
        return;

    worker_ = std::jthread([this](std::stop_token st) { run(st); });
}

void Scheduler::stop() {
    if (!running_.exchange(false))
        return;

    if (worker_.joinable())
        worker_.request_stop();
}

void Scheduler::run(std::stop_token st) {
    while (!st.stop_requested()) {
        const auto now = Clock::now();

        {
            std::lock_guard lk(mutex_);
            for (auto* m : modules_) {
                if (now >= nextRun_[m]) {
                    m->collect();
                    nextRun_[m] = now + m->period();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(501));
    }
}