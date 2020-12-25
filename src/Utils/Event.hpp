#ifndef EVENT_HPP
#define EVENT_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

// singal-slot, observer-subsriber, event-delegate whatever pattern
//
// usage:
//      Event<T...> is an event producer, produces an event when called with Notify or operator()
//
//      EventObserverScopeTyped<T...> is an event observer, connects to one event with Connect/Disconnect.
//      its type signature must match the connected Events type signature.
//      its callback function can be set with SetCallback or in the ctor.
//      it is disconnected from the Event when destructed.
//      destructing an Event disconnects the observer.
//
//      SingleEventObserver and EventObserver are untyped versions of EventObserverScopeTyped,
//      but they cannot be Connected/Disconnected from an Event arbitrarily.
//      they are connected with a callback in Observe, disconnected with the destructor.
//      SingleEventObserver can only subsribe to only one Event at a time.
//      EventObserver can subscribe to multiple events. one EventObserver can only observe an Event once.


template<typename... ARGS>
class Event;


// for EventObserver
class IEventObserver {
public:
    virtual ~IEventObserver () = default;
};


template<typename... ARGS>
class EventObserverScopeTyped : public Noncopyable, public IEventObserver {
public:
    using Func     = std::function<void (ARGS...)>;
    using Callback = std::shared_ptr<Func>;

    friend class Event<ARGS...>;

private:
    Event<ARGS...>* connectedEvent;
    Callback        callback;

public:
    EventObserverScopeTyped ()
        : connectedEvent (nullptr)
        , callback (nullptr)
    {
    }

    EventObserverScopeTyped (const Func& callback)
        : connectedEvent (nullptr)
        , callback (std::make_shared<Func> (callback))
    {
    }

    virtual ~EventObserverScopeTyped () override
    {
        Disconnect ();
    }

    void SetCallback (const Func& newCallback)
    {
        callback = std::make_shared<Func> (newCallback);
    }

    void Connect (Event<ARGS...>& ev);

    void Disconnect ();

private:
    void Notify (ARGS... args)
    {
        if (callback != nullptr) {
            (*callback) (std::forward<ARGS> (args)...);
        }
    }
};

class IEvent {
public:
    virtual ~IEvent () = default;
};

template<typename... ARGS>
class Event : public Noncopyable, public IEvent {
public:
    friend class EventObserverScopeTyped<ARGS...>;

private:
    std::vector<EventObserverScopeTyped<ARGS...>*> evObservers;

public:
    Event () = default;

    virtual ~Event ()
    {
        for (const auto& ob : evObservers) {
            ob->Disconnect ();
        }
    }

    // for observer

    void Connect (EventObserverScopeTyped<ARGS...>& obs, const std::function<void (ARGS...)>& callback)
    {
        obs.SetCallback (callback);
        obs.Connect (*this);
    }

    // for observervable

    void Notify (ARGS... args) const
    {
        for (auto& obv : evObservers) {
            GVK_ASSERT (obv != nullptr);
            obv->Notify (std::forward<ARGS> (args)...);
        }
    }

    void operator() (ARGS... args) const
    {
        Notify (std::forward<ARGS> (args)...);
    }
};


template<typename... ARGS>
void EventObserverScopeTyped<ARGS...>::Connect (Event<ARGS...>& ev)
{
    if (GVK_ERROR (connectedEvent != nullptr)) {
        // must disconnect first
        return;
    }

    ev.evObservers.push_back (this);
    connectedEvent = &ev;
}


template<typename... ARGS>
void EventObserverScopeTyped<ARGS...>::Disconnect ()
{
    if (connectedEvent != nullptr) {
        connectedEvent->evObservers.erase (std::remove (connectedEvent->evObservers.begin (), connectedEvent->evObservers.end (), this), connectedEvent->evObservers.end ());
        connectedEvent = nullptr;
    }
}


class EventObserver {
private:
    std::vector<std::shared_ptr<IEventObserver>> observers;
    std::set<IEvent*>                            observedEvents;

public:
    virtual ~EventObserver () = default;

    template<typename... ARGS, typename Observer>
    void Observe (Event<ARGS...>& ev, Observer observer)
    {
        if (GVK_ERROR (observedEvents.find (&ev) != observedEvents.end ())) {
            // event already observed
            return;
        }

        std::shared_ptr<EventObserverScopeTyped<ARGS...>> obs = std::make_shared<EventObserverScopeTyped<ARGS...>> (observer);
        obs->Connect (ev);
        observers.push_back (std::move (obs));
        observedEvents.insert (&ev);
    }
};

class SingleEventObserver {
private:
    std::shared_ptr<IEventObserver> observer;

public:
    virtual ~SingleEventObserver () = default;

    template<typename... ARGS, typename Observer>
    void Observe (Event<ARGS...>& ev, Observer observerCallback)
    {
        std::shared_ptr<EventObserverScopeTyped<ARGS...>> obs = std::make_shared<EventObserverScopeTyped<ARGS...>> (observerCallback);
        obs->Connect (ev);
        observer = obs;
    }
};


#endif