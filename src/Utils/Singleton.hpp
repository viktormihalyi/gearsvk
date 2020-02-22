#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#define SINGLETON(classname)       \
private:                           \
    classname ()                   \
    {                              \
    }                              \
                                   \
public:                            \
    static classname& Instance ()  \
    {                              \
        static classname instance; \
        return instance;           \
    }

#endif