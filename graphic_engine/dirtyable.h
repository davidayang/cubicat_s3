#ifndef _DIRTY_NOTIFY_POINTER_H_
#define _DIRTY_NOTIFY_POINTER_H_

namespace cubicat {

class Dirtyable {
public:
    void addDirty(bool dirty) {
        m_bDirty |= dirty;
    }
    bool getDirtyAndClear() {
        bool ret = m_bDirty;
        m_bDirty = false;
        return ret;
    }
private:
    bool    m_bDirty = false;
};

template<class T, class U>
class DirtyNotifyPointer
{
public:
    static_assert(std::is_base_of<Dirtyable, T>::value && std::is_base_of<Dirtyable, U>::value, "T, U must be Dirtyable");
    DirtyNotifyPointer(T* ptr, U* owner) : m_ptr(ptr), m_owner(owner) {}
    ~DirtyNotifyPointer() {
        m_owner->addDirty(m_ptr->getDirtyAndClear());
    }

    T* get() const { return m_ptr; }
    T* operator->() const { return m_ptr; }
private:
    T* m_ptr;
    U* m_owner;
};

}

#endif