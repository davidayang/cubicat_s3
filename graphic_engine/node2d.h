#ifndef _NODE_2D_H_
#define _NODE_2D_H_
#include "node.h"

namespace cubicat {

class Node2D;
typedef SharedPtr<Node2D>       Node2DPtr;

class Node2D : public Node {
public:
    DECLARE_RTTI_SUB(Node2D, Node);

    Node2DPtr static create(const string& name = "") {
        return SharedPtr<Node2D>(new Node2D(name));
    }
    virtual void setParent(Node* parent) override;

    void findDrawables(std::vector<DrawablePtr>& outList);
    void setPosition(const Vector2f& pos);
    // [JS_BINDING_BEGIN]
    void rotate(float angle);
    void setPosition(float x, float y);
    void setScale(const Vector2f& scale);
    // [JS_BINDING_END]
    void translate(const Vector2f& dir);
    float getWorldRotation() {return m_worldRot;}
    void setRotation(int16_t angle);

    int16_t getRotation() {return m_localRot;}
    using Node::setScale; // inherit setScale(float)
    void setZ(int z);
    int getZ() {return m_localPos.z;}
    Region getAABB();
protected:
    Node2D(const string& name = "");
    void updateFromParent(bool parentDirty) override;

    float                       m_localRot = 0.0f;
    float                       m_worldRot = 0.0f;
private:
    void attachChild(NodePtr child) override;
    void resort();
};

} // namespace cubicat
#endif