#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <atomic>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <thread>
#include <deque>
#include <condition_variable>

#include "Event/Event.h"

class EventBus
{
public:
    EventBus() = default;
    ~EventBus() {
        stop();
    }

    using HandlerType = std::function<void(const Event::Event&)>;

    void subscribe(HandlerType handler);
    void publish(std::unique_ptr<Event::Event> event) noexcept;

    void start();
    void stop() noexcept;

private:
    void dispatchLoop();

    std::vector<HandlerType> handlers_;
    std::mutex mutex_;
    std::thread worker_thread_;
    std::deque<std::unique_ptr<Event::Event>> event_queue_;
    std::condition_variable cv_;
    
    bool running_{false};
    bool stop_requested_{false};
};


#endif // EVENT_BUS_H