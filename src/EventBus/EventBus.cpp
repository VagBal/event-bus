#include <iostream>
#include "EventBus/EventBus.h"

void EventBus::subscribe(HandlerType handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.emplace_back(std::move(handler));
}

void EventBus::publish(std::unique_ptr<Event::Event> event) noexcept
{
    std::cout << "EventBus publishing event..." << "\n";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        event_queue_.emplace_back(std::move(event));
    }
    cv_.notify_one();
}

void EventBus::start()
{
    std::cout << "EventBus starting..." << "\n";
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) {
        return; // Already running
    }

    stop_requested_ = false;
    running_ = true;
    worker_thread_ = std::thread(&EventBus::dispatchLoop, this);
}

void EventBus::stop() noexcept
{
    std::cout << "EventBus stopping..." << "\n";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return; // Not running
        }
        stop_requested_ = true;
    }
    cv_.notify_one();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    running_ = false;
}

void EventBus::dispatchLoop()
{
    while (true)
    {
        std::unique_ptr<Event::Event> event;
        std::vector<HandlerType> handlers_copy;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { 
                return !event_queue_.empty() || stop_requested_; 
            });

            if (stop_requested_ && event_queue_.empty()) {
                break; // Exit the loop if stop is requested and no events are left
            }

            if (!event_queue_.empty()) {
                event = std::move(event_queue_.front());
                event_queue_.pop_front();
                handlers_copy = handlers_; // Copy handlers to avoid holding the lock during callbacks
            }
        }

        if (event) {
            for (const auto& handler : handlers_copy) {
                handler(*event);
            }
        }
    }
}