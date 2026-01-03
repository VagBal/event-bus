#include <iostream>
#include "EventBus/EventBus.h"

/**
 * @brief Registers a new event handler
 * @param handler Function to be called for each event
 * 
 * Thread-safe registration of event handlers. Handlers are stored in
 * order of subscription and will be invoked in that order for each event.
 */
void EventBus::subscribe(HandlerType handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.emplace_back(std::move(handler));
}

/**
 * @brief Queues an event for asynchronous dispatch
 * @param event Unique pointer to event (ownership transferred)
 * 
 * Adds the event to the queue and notifies the worker thread.
 * Events are dispatched in FIFO order.
 */
void EventBus::publish(std::unique_ptr<Event::Event> event) noexcept
{
    std::cout << "EventBus publishing event..." << "\n";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        event_queue_.emplace_back(std::move(event));
        cv_.notify_one();
    }
}

/**
 * @brief Starts the event dispatching worker thread
 * 
 * Creates and starts the worker thread that processes queued events.
 * Idempotent - calling on already running bus has no effect.
 */
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

/**
 * @brief Stops event dispatching and drains queue
 * 
 * Signals stop to worker thread, waits for all queued events to be
 * processed, then joins the thread. Idempotent.
 */
void EventBus::stop() noexcept
{
    std::cout << "EventBus stopping..." << "\n";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return; // Not running
        }
        stop_requested_ = true;
        cv_.notify_one();
    }

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
}

/**
 * @brief Main event processing loop (runs on worker thread)
 * 
 * Continuously:
 * 1. Waits for events or stop signal
 * 2. Dequeues event and copies handlers
 * 3. Releases lock before invoking handlers
 * 4. Dispatches event to all registered handlers
 * 5. Repeats until stop requested and queue empty
 */
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