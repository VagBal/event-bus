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

/**
 * @class EventBus
 * @brief Thread-safe event dispatching system with asynchronous processing
 * 
 * The EventBus provides a publish-subscribe pattern implementation that:
 * - Processes events asynchronously on a dedicated worker thread
 * - Supports multiple subscribers for each event
 * - Ensures thread-safe event publishing and subscription
 * - Guarantees event ordering (FIFO dispatch)
 * - Gracefully drains all queued events on shutdown
 * 
 * The EventBus must be started with start() before it can dispatch events,
 * and automatically stops in the destructor to ensure clean shutdown.
 * 
 * Thread Safety: All public methods are thread-safe and can be called from
 * multiple threads simultaneously.
 */
class EventBus
{
public:
    /**
     * @brief Default constructor
     */
    EventBus() = default;
    
    /**
     * @brief Destructor - automatically stops the event bus
     * 
     * Ensures all queued events are processed and the worker thread
     * is joined before destruction.
     */
    ~EventBus() {
        stop();
    }

    /**
     * @brief Type alias for event handler functions
     * 
     * Handlers receive a const reference to the base Event class and can
     * use dynamic_cast to check for specific event types.
     */
    using HandlerType = std::function<void(const Event::Event&)>;

    /**
     * @brief Registers a new event handler
     * @param handler Function to be called for each published event
     * 
     * The handler will be invoked on the EventBus worker thread for every
     * event published after subscription. Handlers are called in the order
     * they were subscribed.
     * 
     * Thread Safety: Can be called from any thread
     */
    void subscribe(HandlerType handler);
    
    /**
     * @brief Publishes an event to all subscribers
     * @param event Unique pointer to the event to publish
     * 
     * The event is queued for asynchronous processing. Ownership of the event
     * is transferred to the EventBus. Events are dispatched in FIFO order.
     * 
     * Thread Safety: Can be called from any thread
     * @note This method is noexcept and will not throw exceptions
     */
    void publish(std::unique_ptr<Event::Event> event) noexcept;

    /**
     * @brief Starts the event dispatching worker thread
     * 
     * Must be called before events can be dispatched. Calling start() on
     * an already running EventBus has no effect.
     * 
     * Thread Safety: Can be called from any thread
     */
    void start();
    
    /**
     * @brief Stops the event dispatching and waits for worker thread
     * 
     * Signals the worker thread to stop and waits for all queued events
     * to be processed. After stop() completes, no more events will be
     * dispatched until start() is called again.
     * 
     * Thread Safety: Can be called from any thread
     * @note This method is noexcept and will not throw exceptions
     */
    void stop() noexcept;

private:
    /**
     * @brief Main event dispatch loop running on worker thread
     * 
     * Continuously processes events from the queue until stop is requested.
     * Waits on condition variable when queue is empty.
     */
    void dispatchLoop();

    std::vector<HandlerType> handlers_;                         ///< Registered event handlers
    std::mutex mutex_;                                          ///< Protects shared state
    std::thread worker_thread_;                                 ///< Worker thread for event dispatch
    std::deque<std::unique_ptr<Event::Event>> event_queue_;    ///< FIFO queue of pending events
    std::condition_variable cv_;                                ///< Condition variable for queue notifications
    
    bool running_{false};         ///< Whether EventBus is currently running
    bool stop_requested_{false};  ///< Flag to signal worker thread shutdown
};


#endif // EVENT_BUS_H