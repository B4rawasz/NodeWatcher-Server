#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <functional>
#include <json.hpp>
#include <mutex>

class EventBus {
public:
    using Handler = std::function<void(const message::MessageVariantOUT&)>;

    void subscribe(const Handler& handler);

    void publish(const message::MessageVariantOUT& msg);

private:
    std::vector<Handler> handlers_;
    std::mutex mutex_;
};

#endif  // EVENT_BUS_H