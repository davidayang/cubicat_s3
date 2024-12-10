#ifndef _RIGIDBODYCOMPONENT_H_
#define _RIGIDBODYCOMPONENT_H_

#include "component/component.h"
#include "math/vector2.h"
#include "box2d-lite/Body.h"
#include <cstdint>
#include <vector>
#include <map>
#include <string.h>
#include "message/message_tube.h"

class OrientationListener : protected MessageReceiver {
public:
    OrientationListener();

    void onMessage(int id, const void* msg) override;
};

struct IdentifiedBody : Body {
    int      id;
};

class RigidBodyComponent : public Component, protected MessageReceiver {
public:
    static SharedPtr<RigidBodyComponent> create(uint16_t width, uint16_t height, bool fixed, bool adptiveSize = false) {
        return SharedPtr<RigidBodyComponent>(NEW RigidBodyComponent(width, height, fixed, adptiveSize));
    }
    static SharedPtr<RigidBodyComponent> create(bool fixed) {
        return SharedPtr<RigidBodyComponent>(NEW RigidBodyComponent(fixed));
    }
    ~RigidBodyComponent();

    void onAttachTarget(Node* target) override;
    void onMessage(int id, const void* msg) override;
    void velocity(const Vector2& dir, float speed);
    void update(Node* target, float deltaTime) override;
    void moveTo(const Vector2& pos, float time);
    static void setGravityDir(float dirx, float diry);
    static Vector2 getGravityDir();
    static void initWorld();
    static void tick(float deltaTime);
    static const std::vector<uint32_t>& getColliders(uint32_t id);
private:
    RigidBodyComponent(uint16_t width, uint16_t height, bool fixed, bool adptiveSize = false);
    RigidBodyComponent(bool fixed);
    IdentifiedBody*     m_pRigidBody;
    bool                m_bAdaptiveSize;
    bool                m_bFixed;
    static std::map<uint32_t,std::vector<uint32_t>>  m_sCollisionMap;
    float               m_fMoveTime;
    float               m_fMoveTimeElapse;
    Vector2             m_moveStart;
    Vector2             m_moveDest;
    Vector2             m_size;
};
#endif