#include "rigidbody_component.h"
#include "../node2d.h"
#include <algorithm>
#include <cmath>
#include "../box2d-lite/World.h"
#include "../definitions.h"
#include "../math/vector3.h"

using namespace cubicat;

World* world = nullptr;
OrientationListener oriListener;
std::map<uint32_t,std::vector<uint32_t>> RigidBodyComponent::m_sCollisionMap;
RigidBodyComponent::RigidBodyComponent(uint16_t w, uint16_t h, bool fixed, bool adaptiveSize)
:m_bAdaptiveSize(adaptiveSize),m_bFixed(fixed),m_size(w,h),m_fMoveTime(0),m_fMoveTimeElapse(0)
{
}
RigidBodyComponent::~RigidBodyComponent() {
    if (world) {
        world->Remove(m_pRigidBody);
    }
    delete m_pRigidBody;
    m_pRigidBody = nullptr;
}
void RigidBodyComponent::onMessage(int id, const void* msg) {
    if (id == Msg_SetPos) {
        const Vector2f* pos = (Vector2f*)msg;
        m_pRigidBody->position.x = pos->x;
        m_pRigidBody->position.y = pos->y;
    } else if (id == Msg_MoveTo) {
        MoveToData* move = (MoveToData*)msg;
        moveTo(Vector2f(move->x, move->y), move->time);
    }
}
void RigidBodyComponent::onAttachTarget(Node* target) {
    Component::onAttachTarget(target);
    m_pRigidBody = new IdentifiedBody();
    if (world) {
        const Vector3f& pos = target->getWorldPosition();
        m_pRigidBody->id = target->getId();
        if (m_bFixed) {
            m_pRigidBody->Set(Vec2(m_size.x, m_size.y), FLT_MAX);
        } else { 
            m_pRigidBody->Set(Vec2(m_size.x, m_size.y), 10);
            m_pRigidBody->groupId = 1;
        }
        m_pRigidBody->position.Set(pos.x, pos.y);
        m_pRigidBody->friction = 0.3f;
        world->Add(m_pRigidBody);
    }
    registerLocalMessage(target, Msg_SetPos);
    registerLocalMessage(target, Msg_MoveTo);
}
void RigidBodyComponent::setGravityDir(float dirx, float diry) {
    if (world) {
        world->gravity.x = dirx;
        world->gravity.y = diry;
    }
}
Vector2f RigidBodyComponent::getGravityDir() {
    if (world) {
        return Vector2f(world->gravity.x, world->gravity.y);
    }
    return Vector2f();
}
#define Iterations 6
void RigidBodyComponent::initWorld() {
    if (!world)
        world = new World(Vec2(0, G_VALUE), Iterations);
}
void RigidBodyComponent::update(Node* target, float deltaTime) {
    if (m_bFixed) {
        return;
    }
    if (m_fMoveTime > 0) {
        m_fMoveTimeElapse += deltaTime;
        auto pos = lerp(m_moveStart, m_moveDest, m_fMoveTimeElapse / m_fMoveTime);
        m_pRigidBody->position.x = pos.x;
        m_pRigidBody->position.y = pos.y;
        if (m_fMoveTimeElapse >= m_fMoveTime) {
            m_fMoveTimeElapse = 0;
            m_fMoveTime = 0;
        }
    }
    target->cast<Node2D>()->setPosition(m_pRigidBody->position.x, m_pRigidBody->position.y);
    if (m_bAdaptiveSize) {
        auto aabb = target->cast<Node2D>()->getAABB();
        if (aabb.w > 0 && aabb.h > 0 && m_pRigidBody->width.x != aabb.w && m_pRigidBody->width.y != aabb.h) {
            m_pRigidBody->SetWidth(Vec2(aabb.w,aabb.h));
        }
    }
}
void RigidBodyComponent::moveTo(const Vector2f& pos,float time) {
    m_fMoveTimeElapse = 0;
    m_fMoveTime = time;
    m_moveStart = Vector2f(m_pRigidBody->position.x, m_pRigidBody->position.y);
    m_moveDest = pos;
}
void RigidBodyComponent::velocity(const Vector2f& dir, float speed) {
    m_pRigidBody->velocity.x = dir.x * speed;
    m_pRigidBody->velocity.y = dir.y * speed;
}
void RigidBodyComponent::tick(float deltaTime) {
    if (world) {
        world->Step(deltaTime * 8);
        m_sCollisionMap.clear();
        for (auto itr = world->arbiters.begin();itr != world->arbiters.end();++itr) {
            auto id1 = ((IdentifiedBody*)itr->first.body1)->id;
            auto id2 = ((IdentifiedBody*)itr->first.body2)->id;
            auto itr1 = m_sCollisionMap.find(id1);
            if (itr1 != m_sCollisionMap.end())
                itr1->second.push_back(id2);
            else
                m_sCollisionMap[id1] = std::vector<uint32_t>(id2);
            auto itr2 = m_sCollisionMap.find(id2);
            if (itr2 != m_sCollisionMap.end())
                itr2->second.push_back(id1);
            else
                m_sCollisionMap[id2] = std::vector<uint32_t>(id1);
        }
    }
}
std::vector<uint32_t> emptyVec;
const std::vector<uint32_t>& RigidBodyComponent::getColliders(uint32_t id) {
    auto itr = m_sCollisionMap.find(id);
    if (itr != m_sCollisionMap.end())
        return itr->second;
    return emptyVec;
}

OrientationListener::OrientationListener() {
    registerGlobalMessage(Msg_XYAxisAngle);
}
void OrientationListener::onMessage(int id, const void* msg) {
    if (id == Msg_XYAxisAngle) {
        const Vector2f* xyAxis = (const Vector2f*)msg;
        float rad = ANGLE_2_RAD(-xyAxis->y + 90);
        float x = cosf(rad);
        float y = sinf(rad);
        Vector2f dir(x, y);
        dir.normalize();
        RigidBodyComponent::setGravityDir(dir.x * G_VALUE, dir.y * G_VALUE);
    }
}