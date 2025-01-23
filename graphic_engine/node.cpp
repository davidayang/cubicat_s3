#include "node.h"
#include "definitions.h"
#include <algorithm>

unsigned int Node::m_sIdCounter = 0;

Node::Node(const char* name)
: m_LocalRot(0), m_WorldRot(0), m_LocalScale(Vector2(1, 1)), m_WorldScale(Vector2(1, 1))
, m_pParent(nullptr), m_bVisible(true), m_bDirty(true),m_Z(0),m_name(name)
{
    m_Id = m_sIdCounter;
    m_sIdCounter++;
}

Node::~Node() {
}
void Node::updateFromParent() {
    if (!m_pParent)
        return;
    if (m_bDirty || m_pParent->isDirty()) {
        auto pos = m_LocalPos * m_pParent->getScale();
        auto rot = m_pParent->getRotation() * ANGLE_2_RAD;
        m_WorldPos.x = pos.x * cos(rot) - pos.y * sin(rot);
        m_WorldPos.y = pos.x * sin(rot) + pos.y * cos(rot);
        const Vector2& wPosParent = m_pParent->getWorldPosition();
        auto wRot = m_pParent->getWorldRotation();
        const Vector2& wScale = m_pParent->getWorldScale();
        m_WorldPos += wPosParent;
        m_WorldScale = m_LocalScale * wScale;
        m_WorldRot = m_LocalRot + wRot;
        for (auto& drawable : m_vDrawables) {
            drawable->setRedraw(true);
        }
    }
}
void Node::update(float deltaTime) {
    updateFromParent();
    for (auto& comp : m_vComponents) {
        comp->update(this, deltaTime);
    }
    for (auto& child : m_vChildren) {
        child->update(deltaTime);
    }
    m_bDirty = false;
}
void Node::attachDrawable(DrawablePtr drawable) {
    m_vDrawables.push_back(drawable);
}
void Node::findDrawables(std::vector<DrawablePtr>& outList) {
    if (!isVisible())
        return;
    for (auto d : m_vDrawables) {
        if (!d->isVisible())
            continue;
        d->update(m_WorldPos, m_WorldScale, m_WorldRot);
        outList.push_back(d);
    }
    for (auto child : m_vChildren) {
        child->findDrawables(outList);
    }
}
void Node::addComponent(ComponentPtr component) {
    component->onAttachTarget(this);
    m_vComponents.push_back(component);
}
void Node::setZ(int z) {
    m_Z = z;
    if (m_pParent) {
        m_pParent->resort();
    }
}
Region Node::getAABB() {
    Region aabb;
    for (auto drawable : m_vDrawables) {
        const Vector2& size = drawable->getTextureSize();
        const Vector2& p = drawable->getPivot();
        const Vector2& scale = getWorldScale();
        float x = -size.x * p.x * scale.x;
        float y = size.y * (1 - p.y) * scale.y;
        Region reg(x, y, abs(size.x * scale.x), abs(size.y * scale.y));
        aabb.combine(reg);
    }
    Vector2 p0(aabb.x, aabb.y); 
    Vector2 p1(p0.x+aabb.w, p0.y); 
    Vector2 p2(p1.x, p1.y-aabb.h); 
    Vector2 p3(p0.x, p2.y);
    auto angle = getWorldRotation();
    p0.rotate(angle);
    p1.rotate(angle);
    p2.rotate(angle);
    p3.rotate(angle);
    float minx = std::min(std::min(std::min(p0.x,p1.x),p2.x),p3.x);
    float maxx = std::max(std::max(std::max(p0.x,p1.x),p2.x),p3.x);
    float miny = std::min(std::min(std::min(p0.y,p1.y),p2.y),p3.y);
    float maxy = std::max(std::max(std::max(p0.y,p1.y),p2.y),p3.y);
    aabb.x = minx + m_WorldPos.x;
    aabb.y = maxy + m_WorldPos.y;
    aabb.w = (uint16_t)(maxx - minx);
    aabb.h = (uint16_t)(maxy - miny);
    return aabb;
}
const std::vector<DrawablePtr>& Node::getDrawables() const {
    return m_vDrawables;
}
DrawablePtr Node::getDrawable(uint32_t index) {
    if (index >= m_vDrawables.size())
        return SharedPtr<Drawable>(nullptr);
    return m_vDrawables[index];
}

void Node::clearDrawables() {
    m_vDrawables.clear();
}