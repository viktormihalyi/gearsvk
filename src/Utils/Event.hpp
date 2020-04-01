#ifndef EVENT_HPP
#define EVENT_HPP

#include <functional>
#include <utility>
#include <vector>

template<typename... ARGS>
class Event {
public:
    using Func     = std::function<void (ARGS...)>;
    using Callback = std::shared_ptr<Func>;

    std::vector<Callback> observers;

    inline void Notify (ARGS... args) const
    {
        for (auto& ob : observers) {
            (*ob) (std::forward<ARGS> (args)...);
        }
    }

    inline void operator() (ARGS... args) const
    {
        for (auto& ob : observers) {
            (*ob) (std::forward<ARGS> (args)...);
        }
    }

    template<class Functor>
    inline Callback CreateDelegate (Functor f)
    {
        return Callback (new Func (f));
    }

    template<class Functor>
    inline void operator+= (Functor f)
    {
        observers.push_back (CreateDelegate (f));
    }
};

#endif