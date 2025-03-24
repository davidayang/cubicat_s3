#include "node2d.h"
#include "definitions.h"
#include <algorithm>

using namespace cubicat;
Node2D::Node2D(const string& name)
: Node(name)
{
}

void Node2D::updateFromParent(bool parentDirty) {
    if (!m_pParent)
        return;
    Node::updateFromParent(parentDirty);
    auto parent = m_pParent->cast<Node2D>();
    if (isTransformDirty() || parentDirty) {
        auto pos = m_localPos * parent->getScale();
        auto rot = ANGLE_2_RAD(parent->getRotation());
        m_worldPos.x = pos.x * cos(rot) - pos.y * sin(rot);
        m_worldPos.y = pos.x * sin(rot) + pos.y * cos(rot);
        const Vector3f& wPosParent = m_pParent->getWorldPosition();
        auto wRot = parent->getWorldRotation();
        const Vector3f& wScale = m_pParent->getWorldScale();
        m_worldPos += wPosParent;
        m_worldScale = m_localScale * wScale;
        m_worldRot = m_localRot + wRot;
    }
}
void Node2D::setParent(Node* parent) {
    if (parent) {
        auto node2d = parent->cast<Node2D>();
        if (!node2d) {
            return;
        }
    }
    Node::setParent(parent);
}

void Node2D::findDrawables(std::vector<DrawablePtr>& outList) {
    if (!isVisible())
        return;
    for (auto d : m_vDrawables) {
        if (!d->isVisible())
            continue;
        d->update(Vector2f(m_worldPos.x, m_worldPos.y), Vector2f(m_worldScale.x, m_worldScale.y), m_worldRot);
        outList.push_back(d);
    }
    for (auto child : m_vChildren) {
        child->cast<Node2D>()->findDrawables(outList);
    }
}

void Node2D::setZ(int z) {
    m_localPos.z = z;
    if (m_pParent) {
        m_pParent->cast<Node2D>()->resort();
    }
}
Region Node2D::getAABB() {
    Region aabb;
    for (auto drawable : m_vDrawables) {
        const Vector2us& size = drawable->getMaterial()->getTexture()->getTextureSize();
        const Vector2f& p = drawable->getPivot();
        const Vector3f& scale = getWorldScale();
        float x = -size.x * p.x * scale.x;
        float y = size.y * (1 - p.y) * scale.y;
        Region reg(x, y, abs(size.x * scale.x), abs(size.y * scale.y));
        aabb.combine(reg);
    }
    Vector2f p0(aabb.x, aabb.y); 
    Vector2f p1(p0.x+aabb.w, p0.y); 
    Vector2f p2(p1.x, p1.y-aabb.h); 
    Vector2f p3(p0.x, p2.y);
    auto angle = getWorldRotation();
    p0.rotate(angle);
    p1.rotate(angle);
    p2.rotate(angle);
    p3.rotate(angle);
    float minx = std::min(std::min(std::min(p0.x,p1.x),p2.x),p3.x);
    float maxx = std::max(std::max(std::max(p0.x,p1.x),p2.x),p3.x);
    float miny = std::min(std::min(std::min(p0.y,p1.y),p2.y),p3.y);
    float maxy = std::max(std::max(std::max(p0.y,p1.y),p2.y),p3.y);
    aabb.x = minx + m_worldPos.x;
    aabb.y = maxy + m_worldPos.y;
    aabb.w = (uint16_t)(maxx - minx);
    aabb.h = (uint16_t)(maxy - miny);
    return aabb;
}

void Node2D::attachChild(NodePtr child) {
    Node::attachChild(child);
    resort();
}


bool zCompare(const NodePtr& a, const NodePtr& b) {
    return a->cast<Node2D>()->getZ() < b->cast<Node2D>()->getZ();
}

void Node2D::resort() {
    // resort z order
    std::sort(m_vChildren.begin(), m_vChildren.end(), zCompare);
}
void Node2D::setRotation(int16_t angle) {
    m_localRot = angle;
    setTransformDirty(true);
}
void Node2D::rotate(float angle) {
    m_localRot += angle;
    setTransformDirty(true);
}
void Node2D::translate(const Vector2f& dir) {
    m_localPos.x += dir.x;
    m_localPos.y += dir.y;
    setTransformDirty(true);
}
void Node2D::setPosition(const Vector2f& pos) {
    setPosition(pos.x, pos.y);
}
void Node2D::setPosition(float x,float y) {
    m_localPos.x = x;
    m_localPos.y = y;
    setTransformDirty(true);
}
void Node2D::setScale(const Vector2f& scale) {
    m_localScale.x = scale.x;
    m_localScale.y = scale.y;
    setTransformDirty(true);
}