#ifndef EVENT_EVENT_H
#define EVENT_EVENT_H

namespace Event
{

/**
 * @struct Event
 * @brief Base class for all events in the event-driven system
 * 
 * This is the abstract base class that all event types must inherit from.
 * It provides a virtual destructor to ensure proper cleanup of derived classes
 * when events are handled through base class pointers.
 * 
 * Events are typically created on the heap and passed as unique_ptr to the EventBus
 * for dispatching to registered handlers.
 */
struct Event
{
    /**
     * @brief Virtual destructor for proper polymorphic deletion
     */
    virtual ~Event() = default;
};

} // namespace Event

#endif // EVENT_EVENT_H