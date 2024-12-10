#ifndef _RTTI_H_
#define _RTTI_H_


struct RTTI
{
    template <typename T>
    static const void* const get()
    {
        return &impl<T>::id; // impl<T>::id will have a unique address for every T.
    }
    template <typename T>
    bool isA() const {
        return getTypeId() == RTTI::get<T>();
    }
    template <typename T>
    bool ofA() const {
        return _ofType(RTTI::get<T>());
    }
    template <typename T>
    T* cast() {
        if (ofA<T>()) {
            return static_cast<T*>(this);
        } else {
            return nullptr;
        }
    }
    virtual bool _ofType(const void* type) const = 0;
private:
    virtual const void* const getTypeId() const = 0;

    template <typename T> 
    struct impl { static const T* const id; };
    template <typename T> struct impl<const T> : impl<T> {};
    template <typename T> struct impl<T*> : impl<T> {};
};
template <typename T> 
const T* const RTTI::impl<T>::id = nullptr;

#define DECLARE_RTTI_ROOT(T) \
    virtual const void* const getTypeId() const override { \
        return RTTI::get<T>(); \
    } \
    virtual bool _ofType(const void* type) const override { \
        return T::getTypeId() == type; \
    }

#define DECLARE_RTTI_SUB(T, Base) \
    virtual const void* const getTypeId() const override { \
        return RTTI::get<T>(); \
    } \
    virtual bool _ofType(const void* type) const override { \
        if (T::getTypeId() == type) { \
            return true; \
        } \
        return Base::_ofType(type); \
    }

#endif