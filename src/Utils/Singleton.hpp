#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#define SINGLETON(classname)             \
private:                                 \
    static classname* instance;          \
    classname ()                         \
    {                                    \
    }                                    \
                                         \
public:                                  \
    static classname* GetInstance ()     \
    {                                    \
        if (instance == nullptr) {       \
            instance = new classname (); \
        }                                \
        return instance;                 \
    }

#endif