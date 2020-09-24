#ifndef EVENT_HPP
#define EVENT_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

class ObserverHandle;


class Detachable {
public:
    virtual void Detach (ObserverHandle&) = 0;
};

USING_PTR (ObserverHandle);
class ObserverHandle : public Noncopyable {
    USING_CREATE (ObserverHandle);

public:
    using HandleType = uintptr_t;

private:
    Detachable* observable;
    HandleType  handle;

public:
    ObserverHandle ()
        : observable (nullptr)
        , handle (0)
    {
    }

    ObserverHandle (Detachable& detachable, HandleType handle)
        : observable (&detachable)
        , handle (handle)
    {
    }

    ~ObserverHandle ()
    {
        // TODO
        //Detach ();
    }

    ObserverHandle (ObserverHandle&& other) noexcept
        : observable (other.observable)
        , handle (other.handle)
    {
        other.observable = nullptr;
        other.handle     = 0;
    }

    void operator= (ObserverHandle&& other) noexcept
    {
        observable       = other.observable;
        handle           = other.handle;
        other.observable = nullptr;
        other.handle     = 0;
    }

    void Detach ()
    {
        if (observable != nullptr) {
            observable->Detach (*this);
            observable = nullptr;
            handle     = 0;
        }
    }

    operator HandleType () const { return handle; }
};

template<typename... ARGS>
class Event;

class IEventObserver {
public:
    virtual ~IEventObserver () = default;
};

template<typename... ARGS>
class EventObserver : public Noncopyable, public IEventObserver {
private:
    using Func     = std::function<void (ARGS...)>;
    using Callback = std::shared_ptr<Func>;

    Event<ARGS...>* connectedEvent;
    Callback        callback;

public:
    EventObserver ()
        : connectedEvent (nullptr) 
        , callback (nullptr)
    {
    }

    EventObserver (const Func& callback)
        : connectedEvent (nullptr)
        , callback (std::make_shared<Func> (callback))
    {
    }

    virtual ~EventObserver () override
    {
        Disconnect ();
    }

    void SetCallback (const Func& newCallback)
    {
        callback = std::make_shared<Func> (newCallback);
    }

    void Connect (Event<ARGS...>& ev);
    void Disconnect ();

    void Notify (ARGS&&... args)
    {
        if (callback != nullptr) {
            (*callback) (std::forward<ARGS> (args)...);
        }
    }
};

template<typename... ARGS>
class Event : public Noncopyable, public Detachable {
public:
    using Func     = std::function<void (ARGS...)>;
    using Callback = std::shared_ptr<Func>;

private:
    std::vector<Callback> observers;

public:
    std::vector<EventObserver<ARGS...>*> evObservers;

public:
    Event () = default;

    virtual ~Event ()
    {
        for (const auto& ob : evObservers) {
            ob->Disconnect ();
        }
    }

    template<class Functor>
    static inline Callback CreateCallback (Functor f)
    {
        return Callback (new Func (f));
    }


    // for observervable

    inline void Notify (ARGS... args) const
    {
        for (auto& ob : observers) {
            GVK_ASSERT (ob != nullptr);
            (*ob) (std::forward<ARGS> (args)...);
        }
        for (auto& obv : evObservers) {
            GVK_ASSERT (obv != nullptr);
            obv->Notify (std::forward<ARGS> (args)...);
        }
    }

    inline void operator() (ARGS... args) const
    {
        Notify (std::forward<ARGS> (args)...);
    }


    // for observer

    template<class Functor>
    inline ObserverHandle operator+= (Functor f)
    {
        Callback c = CreateCallback (f);
        observers.push_back (c);
        return ObserverHandle (*this, reinterpret_cast<ObserverHandle::HandleType> (c.get ()));
    }

    template<class Functor>
    inline ObserverHandle operator= (Functor f)
    {
        observers.clear ();
        return (*this) += f;
    }

    inline ObserverHandle operator+= (Callback f)
    {
        GVK_ASSERT (f != nullptr);
        observers.push_back (f);
        return ObserverHandle (*this, reinterpret_cast<ObserverHandle::HandleType> (f.get ()));
    }

    inline ObserverHandle operator= (Callback f)
    {
        observers.clear ();
        return (*this) += f;
    }

    inline void Detach (ObserverHandle& f)
    {
        auto RemovePred = [&] (const Callback& c) {
            return reinterpret_cast<ObserverHandle::HandleType> (c.get ()) == f;
        };

        observers.erase (std::remove_if (observers.begin (), observers.end (), RemovePred), observers.end ());
    }

    inline void operator-= (Callback f)
    {
        observers.erase (std::remove (observers.begin (), observers.end (), f), observers.end ());
    }
};


template<typename... ARGS>
void EventObserver<ARGS...>::Connect (Event<ARGS...>& ev)
{
    if (GVK_ERROR (connectedEvent != nullptr)) {
        // must disconnect first
        return;
    }

    ev.evObservers.push_back (this);
    connectedEvent = &ev;
}

template<typename... ARGS>
void EventObserver<ARGS...>::Disconnect ()
{
    if (connectedEvent != nullptr) {
        connectedEvent->evObservers.erase (std::remove (connectedEvent->evObservers.begin (), connectedEvent->evObservers.end (), this), connectedEvent->evObservers.end ());
        connectedEvent = nullptr;
    }
}


class EventObserverClass {
private:
    std::vector<std::shared_ptr<IEventObserver>> observers;

public:
    virtual ~EventObserverClass () = default;

    template<typename... ARGS, typename Observer>
    void Observe (Event<ARGS...>& ev, Observer observer)
    {
        std::shared_ptr<EventObserver<ARGS...>> obs = std::make_shared<EventObserver<ARGS...>> (observer);
        obs->Connect (ev);
        observers.push_back (std::move (obs));
    }
};


#endif