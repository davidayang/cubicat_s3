#ifndef _SHARED_POINTER_H_
#define _SHARED_POINTER_H_
#include <memory>

typedef void (*Deleter)(void*);

using std::static_pointer_cast;
using namespace std;
template<class T>
class SharedPtr : public shared_ptr<T> {
public:
    explicit SharedPtr(T* ptr = nullptr): shared_ptr<T>(ptr) {}
    explicit SharedPtr(T* ptr, Deleter deleter): shared_ptr<T>(ptr, deleter) {}
    // 拷贝构造函数
    SharedPtr(const SharedPtr& other) : shared_ptr<T>(other) {};
    // 从 std::shared_ptr 构造
    SharedPtr(const std::shared_ptr<T>& other) : std::shared_ptr<T>(other) {}
    template <typename... Args>
    static SharedPtr<T> make_shared(Args&&... args) {
        // 使用 std::make_shared 创建对象
        auto rawPtr = std::make_shared<T>(std::forward<Args>(args)...);
        // 创建 SharedPtr 并返回
        return SharedPtr<T>(rawPtr);
    }
    template< class Y>
    explicit SharedPtr(Y* ptr) : shared_ptr<T>(ptr) {}
    template< class Y>
    SharedPtr(const SharedPtr<Y>& r) : shared_ptr<T>(r) {}
    //constructor with deleter
    template< class Y >
    SharedPtr( Y* ptr, Deleter d ) : shared_ptr<T>(ptr, d) {}
    
    // 赋值操作符
    SharedPtr<T>& operator=(const SharedPtr<T>& rhs) {
        shared_ptr<T>::operator=(rhs);
        return *this;
    }
    // // 比较操作符
    // friend bool operator==(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs) {
    //     return shared_ptr<T>::operator==(rhs);
    // }
    // 成员访问操作符
    T* operator->() const {
         return shared_ptr<T>::operator->();
    }
    // 重载 operator T*() 进行隐式转换
    operator T*() const
    {
        return this->get();
    }
};

#endif