#ifndef _NODE_H_
#define _NODE_H_
#include "math/vector2.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include "drawable/drawable.h"
#include "component/component.h"
#include "message/message_tube.h"
#include "region.h"
#include "core/memory_object.h"
#include <string.h>

class Node;
typedef SharedPtr<Node>       NodePtr;

class Node : public std::enable_shared_from_this<Node>,public MessageTube, public RTTI, public MemoryObject{
public:
    DECLARE_RTTI_ROOT(Node);
    virtual ~Node();

    NodePtr static create(const char* name = "") {
        return SharedPtr<Node>(NEW Node(name));
    }
    unsigned int getId() {return m_Id;}
    void setName(const char* name) {m_name = name;}
    const std::string& getName() {return m_name;} 
    void setPosition(const Vector2& pos);
    void setPosition(float x, float y);
    void translate(const Vector2& dir);
    const Vector2& getPosition();
    const Vector2& getWorldPosition();
    const Vector2& getScale();
    const Vector2& getWorldScale();
    float getWorldRotation();
    void setRotation(int16_t angle);
    void rotate(float angle);
    int16_t getRotation();
    void setParent(Node* parent);
    void setScale(const Vector2& scale);
    void setScale(float x, float y);
    void setZ(int z);
    int getZ() {return m_Z;}
    Node* getParent() {return m_pParent;}
    void update(float deltaTime);
    void attachDrawable(DrawablePtr drawable);
    const std::vector<DrawablePtr>& getDrawables() {return m_vDrawables;}
    void clearDrawables() {m_vDrawables.clear();}
    void findDrawables(std::vector<DrawablePtr>& outList);
    const std::vector<NodePtr> getChildren();
    void setVisible(bool visible) {m_bVisible = visible;}
    bool isVisible() {return m_bVisible;}
    bool isDirty() {return m_bDirty;}
    void addComponent(ComponentPtr component);
    int getComponentCount() {return (int)m_vComponents.size();}
    Region getAABB();
protected:
    Node(const char* name = "");
    virtual void onSetParent(){};
private:
    void attachChild(NodePtr child);
    void detachChild(NodePtr child);
    void updateFromParent();
    void resort();
    Vector2 m_LocalPos;
    int     m_Z;
    float   m_LocalRot;
    Vector2 m_LocalScale;

    Vector2 m_WorldPos;
    float   m_WorldRot;
    Vector2 m_WorldScale;
    std::vector<NodePtr>  m_vChildren;
    std::vector<DrawablePtr>    m_vDrawables;
    std::vector<ComponentPtr>   m_vComponents;
    Node*                       m_pParent;
    std::string                      m_name;
    unsigned int                m_Id;
    static unsigned int         m_sIdCounter;
    bool                        m_bVisible;
    bool                        m_bDirty;
};

inline void Node::setPosition(const Vector2& pos) {
    setPosition(pos.x, pos.y);
}
inline void Node::setPosition(float x,float y) {
    m_LocalPos.x = x;
    m_LocalPos.y = y;
    m_bDirty = true;
}
inline void Node::translate(const Vector2& dir) {
    m_LocalPos.x += dir.x;
    m_LocalPos.y += dir.y;
    m_bDirty = true;
}
inline const Vector2& Node::getPosition() {
    return m_LocalPos;
}
inline const Vector2& Node::getWorldPosition() {
    if (m_bDirty) {
        updateFromParent();
    }
    return m_WorldPos;
}
inline const Vector2& Node::getScale() {
    return m_LocalScale;
}
inline void Node::setScale(const Vector2& scale) {
    setScale(scale.x, scale.y);
}
inline const Vector2& Node::getWorldScale(){
    return m_WorldScale;
}
inline float Node::getWorldRotation() {
    return m_WorldRot;
}
inline void Node::setScale(float x, float y) {
    m_LocalScale.x = x;
    m_LocalScale.y = y;
    m_bDirty = true;
}
inline void Node::setRotation(int16_t angle) {
    m_LocalRot = angle;
    m_bDirty = true;
}
inline void Node::rotate(float angle) {
    m_LocalRot += angle;
    m_bDirty = true;
}
inline int16_t Node::getRotation() {
    return m_LocalRot;
}
inline void Node::setParent(Node* parent) {
    if (parent == this || parent == m_pParent)
        return;
    if (m_pParent)
        m_pParent->detachChild(shared_from_this());
     m_pParent = parent;
    if (m_pParent)
        m_pParent->attachChild(shared_from_this());
    m_bDirty = true;
    onSetParent();
}
inline bool zCompare(const NodePtr& a, const NodePtr& b) {
    return a->getZ() < b->getZ();
}
inline void Node::attachChild(NodePtr child) {
    m_vChildren.push_back(child);
    resort();
}
inline void Node::resort() {
    // resort z order
    std::sort(m_vChildren.begin(), m_vChildren.end(), zCompare);
}
inline void Node::detachChild(NodePtr child) {
    for (auto it = m_vChildren.begin(); it != m_vChildren.end(); ++it) {
        if (*it == child) {
            m_vChildren.erase(it);
            break;
        }
    }
}
inline const std::vector<NodePtr> Node::getChildren() {
    return m_vChildren;
}

#endif