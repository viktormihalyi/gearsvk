#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP

#include <utility>
#include <vector>

template<typename ObserverType>
class ObservableBase {
protected:
    std::vector<ObserverType*> observers;

public:
    virtual ~ObservableBase () {}

    void Attach (ObserverType* observer)
    {
        observers.push_back (observer);
    }

    void Detach (ObserverType* observer)
    {
        observers.push_back (observer);
    }
};


#define OBSERVABLE_NOTIFY(fn)                          \
    template<typename... ARGS>                         \
    void Notify##fn (ARGS&&... args)                   \
    {                                                  \
        for (auto ob : observers) {                    \
            ob->On##fn (std::forward<ARGS> (args)...); \
        }                                              \
    }

#endif