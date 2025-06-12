#ifndef _NODE_H_
#define _NODE_H_
#include "math/vector2.h"
#include "math/vector3.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include "drawable/drawable.h"
#include "component/component.h"
#include "core/message/message_tube.h"
#include "region.h"
#include "core/memory_object.h"
#include "core/id_object.h"
#include <string.h>

namespace cubicat {


class Node;
typedef SharedPtr<Node>     NodePtr;

class Node : public std::enable_shared_from_this<Node>, public MessageTube, public MessageDispatcher, public RTTI, public MemoryObject, public IDObject{
public:
    DECLARE_RTTI_ROOT(Node);
    virtual ~Node();
    // [JS_BINDING_BEGIN]
    virtual void setParent(Node* parent);
    // [JS_BINDING_END]
    // @param deltaTime update time elapse in second
    // @param parentDirty parent object dirty mark, including all it's ancestors
    virtual void update(float deltaTime,bool parentDirty);

    // [JS_BINDING_BEGIN]
    void setName(const string& name) {m_name = name;}
    void setVisible(bool visible);
    void setScale(float s);
    // [JS_BINDING_END]
    const std::string& getName() {return m_name;} 
    const Vector3f& getPosition() {return m_localPos;}
    Vector3f getWorldPosition();
    const Vector3f& getScale() {return m_localScale;}
    const Vector3f& getWorldScale();
    Node* getParent() {return m_pParent;}
    void attachDrawable(DrawablePtr drawable);
    const std::vector<DrawablePtr>& getDrawables() const;
    // [JS_BINDING_BEGIN]
    DrawablePtr getDrawable(uint32_t index);
    // [JS_BINDING_END]
    template<class T>
    T* getDrawable();
    void clearDrawables();
    NodePtr getChild(uint32_t index);
    template<class T>
    T* getChild();
    const std::vector<NodePtr>& getChildren();
    bool isVisible() {return m_bVisible;}
    bool isTransformDirty() {return m_bTransformDirty;}
    void addComponent(ComponentPtr component);
    int getComponentCount() {return (int)m_vComponents.size();}
protected:
    Node(const string& name = "");
    virtual void updateFromParent(bool parentDirty);
    virtual void onSetParent(){};
    virtual void attachChild(NodePtr child);
    void setTransformDirty(bool dirty);

    Node*                       m_pParent = nullptr;
    std::vector<ComponentPtr>   m_vComponents;

    Vector3f                    m_localPos;
    Vector3f                    m_localScale = Vector3f(1.0f, 1.0f, 1.0f);
    Vector3f                    m_worldPos;
    Vector3f                    m_worldScale = Vector3f(1.0f, 1.0f, 1.0f);

    std::vector<NodePtr>        m_vChildren;
    std::vector<DrawablePtr>    m_vDrawables;
private:
    void detachChild(NodePtr child);

    std::string                 m_name;
    static unsigned int         m_sIdCounter;
    bool                        m_bVisible = true;
    bool                        m_bTransformDirty = true;
};

inline Vector3f Node::getWorldPosition() {
    if (m_bTransformDirty) {
        updateFromParent(false);
    }
    return m_worldPos;
}

inline const Vector3f& Node::getWorldScale(){
    return m_worldScale;
}

inline void Node::attachChild(NodePtr child) {
    m_vChildren.push_back(child);
}

inline void Node::detachChild(NodePtr child) {
    for (auto it = m_vChildren.begin(); it != m_vChildren.end(); ++it) {
        if (*it == child) {
            m_vChildren.erase(it);
            break;
        }
    }
}
inline const std::vector<NodePtr>& Node::getChildren() {
    return m_vChildren;
}

template<class T>
T* Node::getDrawable() {
    for (auto& drawable : m_vDrawables) {
        auto d = drawable->cast<T>();
        if (d)
            return d;
    }
    return nullptr;
}

template<class T>
T* Node::getChild() {
    for (auto& child : m_vChildren) {
        auto d = child->cast<T>();
        if (d)
            return d;
    }
    return nullptr;
}

}
#endif