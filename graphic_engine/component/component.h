#ifndef _COMPONENT_H_
#define _COMPONENT_H_
#include "core/shared_pointer.h"
#include "core/memory_object.h"

class Node;
class Component : public MemoryObject
{
    friend class Node;
public:
    virtual ~Component(){}
protected:
    virtual void onAttachTarget(Node* target){};
    virtual void update(Node* target,float deltaTime) = 0;
};
typedef SharedPtr<Component>       ComponentPtr;
#endif