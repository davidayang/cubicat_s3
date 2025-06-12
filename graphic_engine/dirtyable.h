#ifndef _DIRTY_NOTIFY_POINTER_H_
#define _DIRTY_NOTIFY_POINTER_H_

namespace cubicat {

class Dirtyable {
public:
    void markDirty() {
        m_bDirty = true;
    }
    bool getDirtyAndClear() {
        bool dirty = m_bDirty;
        m_bDirty = false;
        return dirty;
    }
private:
    bool    m_bDirty = true;
};

template<class T, class U>
class DirtyNotifyPointer
{
public:
    static_assert(std::is_base_of<Dirtyable, T>::value && std::is_base_of<Dirtyable, U>::value, "T, U must be Dirtyable");
    DirtyNotifyPointer(T* ptr, U* owner) : m_ptr(ptr), m_owner(owner) {}
    ~DirtyNotifyPointer() {
        if (m_ptr->getDirtyAndClear())
            m_owner->markDirty();
    }

    T* get() const { return m_ptr; }
    T* operator->() const { return m_ptr; }
private:
    T* m_ptr;
    U* m_owner;
};

}

#endif