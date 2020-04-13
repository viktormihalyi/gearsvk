#ifndef EVENT_HPP
#define EVENT_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"

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


class ObserverHandle : public Noncopyable {
public:
    using HandleType = uintptr_t;

private:
    Detachable& observable;
    HandleType  handle;

public:
    ObserverHandle (Detachable& observable, HandleType handle)
        : observable (observable)
        , handle (handle)
    {
        if (&observable == nullptr) {
            throw std::runtime_error ("event is nullptr");
        }
        if (handle == 0) {
            throw std::runtime_error ("handle is nullptr");
        }
    }

    void Detach ()
    {
        observable.Detach (*this);
    }

    operator HandleType () const { return handle; }
};


template<typename... ARGS>
class Event : public Noncopyable, public Detachable {
public:
    using Func     = std::function<void (ARGS...)>;
    using Callback = std::shared_ptr<Func>;

private:
    std::vector<Callback> observers;

public:
    virtual ~Event () = default;

    template<class Functor>
    static inline Callback CreateCallback (Functor f)
    {
        return Callback (new Func (f));
    }


    // for observervable

    inline void Notify (ARGS... args) const
    {
        for (auto& ob : observers) {
            ASSERT (ob != nullptr);
            (*ob) (std::forward<ARGS> (args)...);
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

    inline ObserverHandle operator+= (Callback f)
    {
        ASSERT (f != nullptr);
        observers.push_back (f);
        return ObserverHandle (*this, reinterpret_cast<ObserverHandle::HandleType> (f.get ()));
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


#endif