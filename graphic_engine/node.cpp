#include "node.h"
#include "definitions.h"
#include <algorithm>

using namespace cubicat;


Node::Node(const string& name)
: m_name(name)
{
}

Node::~Node() {
}

void Node::setParent(Node* parent) {
    if (parent == this || parent == m_pParent)
        return;
    if (m_pParent) {
        m_pParent->detachChild(shared_from_this());
    }
    m_pParent = parent;
    if (m_pParent)
        m_pParent->attachChild(shared_from_this());
    setTransformDirty(true);
    onSetParent();
}
void Node::update(float deltaTime, bool parentDirty) {
    if (!isVisible())
        return;
    parentDirty = parentDirty || isTransformDirty();
    updateFromParent(parentDirty);
    setTransformDirty(false);
    for (auto& comp : m_vComponents) {
        comp->update(this, deltaTime);
    }
    for (auto& child : m_vChildren) {
        child->update(deltaTime, parentDirty);
    }
}
void Node::updateFromParent(bool parentDirty) {
    if (parentDirty || isTransformDirty()) {
        for (auto& drawable : m_vDrawables) {
            drawable->markDirty();
        }
    }
}
void Node::attachDrawable(DrawablePtr drawable) {
    m_vDrawables.push_back(drawable);
}

void Node::addComponent(ComponentPtr component) {
    component->onAttachTarget(this);
    m_vComponents.push_back(component);
}
void Node::setVisible(bool visible) {
    if (m_bVisible != visible) {
        m_bVisible = visible;
        for (auto& drawable : m_vDrawables) {
            drawable->setVisible(visible);
        }
    }
}
void Node::setScale(float s) {
    setTransformDirty(true);
    m_localScale.x = s;
    m_localScale.y = s;
    m_localScale.z = s;
}
const std::vector<DrawablePtr>& Node::getDrawables() const {
    return m_vDrawables;
}
DrawablePtr Node::getDrawable(uint32_t index) {
    if (index >= m_vDrawables.size())
        return DrawablePtr(nullptr);
    return m_vDrawables[index];
}

void Node::clearDrawables() {
    m_vDrawables.clear();
}

NodePtr Node::getChild(uint32_t index) {
    if (index >= m_vChildren.size())
        return NodePtr(nullptr);
    return m_vChildren[index];
}

void Node::setTransformDirty(bool dirty) {
    m_bTransformDirty = dirty;
}