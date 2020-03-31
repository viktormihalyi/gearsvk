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

    void Notify (ARGS&&... args) const
    {
        for (auto& ob : observers) {
            (*ob) (std::forward<ARGS> (args)...);
        }
    }

    template<class Functor>
    Callback CreateDelegate (Functor f)
    {
        return Callback (new Func (f));
    }

    template<class Functor>
    void operator+= (Functor f)
    {
        observers.push_back (CreateDelegate (f));
    }
};

#endif