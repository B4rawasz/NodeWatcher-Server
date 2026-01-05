#include <event_bus.h>
#include <vector>

void EventBus::subscribe(const Handler& handler) {
    std::lock_guard lk(mutex_);
    handlers_.push_back(std::move(handler));
}

void EventBus::publish(const message::MessageVariantOUT& msg) {
    std::vector<Handler> handlersCopy;
    {
        std::lock_guard lk(mutex_);
        handlersCopy = handlers_;
    }

    for (const auto& handler : handlersCopy) {
        handler(msg);
    }
}