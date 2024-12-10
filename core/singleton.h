#ifndef _SINGLETON_H_
#define _SINGLETON_H_

template <typename T>
class Singleton {
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
protected:
    Singleton() {}
    virtual ~Singleton() {}
private:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

#endif