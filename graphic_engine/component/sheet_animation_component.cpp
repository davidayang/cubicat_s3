#include <cmath>
#include "sheet_animation_component.h"
#include "../node.h"
#include "../math/vector3.h"
using namespace cubicat;

SheetAnimationComponent::SheetAnimationComponent()
{
    m_currentMountingPoint.len = 0;
    m_currentMountingPoint.data = nullptr;
}
void SheetAnimationComponent::onAttachTarget(Node* target) {
    registerLocalMessage(target, Msg_PlayAnim);
}
void SheetAnimationComponent::update(Node* target, float deltaTime) {
    auto texture = target->getDrawable(0)->getMaterial()->getTexture();
    if (m_pCurrentAnimData && texture.get()) {
        m_timer.update(deltaTime * m_fSpeed);
        int count = m_pCurrentAnimData->indices.size();
        int index = (int)(m_timer.getElapse() / m_pCurrentAnimData->interval);
        if (m_timer.getLoopType() == Timer::LOOP)
            index = index % count;
        if (index < 0) {
            index = count - index;
        }
        if (index >= count)
            return;
        // if (index < m_currentMountingPoint.len) {
        //     int offset = index * 3;
        //     auto& pivot = m_pSheet->getPivot();
        //     int xoffset = m_pSheet->getTextureSize().x * pivot.x;
        //     int yoffset = m_pSheet->getTextureSize().y * pivot.y;
        //     Vector3h mpt(m_currentMountingPoint.data[offset] - xoffset,
        //      m_currentMountingPoint.data[offset+1] - yoffset, m_currentMountingPoint.data[offset+2]);
        //     target->broadcastMessage(Msg_MountingPointPos, &mpt);
        // }
        uint8_t frameIndex = m_pCurrentAnimData->indices.at(index);
        texture->setFrame(frameIndex);
    }
}
void SheetAnimationComponent::setAnimation(const char*  animName, std::vector<unsigned char> indices,float interval, bool loop) {
    SheetAnimData data = SheetAnimData();
    data.interval = interval;
    data.indices = indices;
    m_timer.setLoopType(loop?Timer::LOOP:Timer::ONCE);
    m_Anims.emplace(animName, data);
}

void SheetAnimationComponent::playAnimation(const char* animName, float speed) {
    auto itr = m_Anims.find(animName);
    if (itr != m_Anims.end()) {
        m_pCurrentAnimData = &itr->second;
        if (m_currentAnim == animName && m_timer.getLoopType() == Timer::LOOP) {
            return;
        }
        m_currentAnim = animName;
        m_fSpeed = speed;
        m_timer.reset();
    }
    // auto mptItr = m_mountingPointData.find(animName);
    // if (mptItr != m_mountingPointData.end()) {
    //     m_currentMountingPoint = mptItr->second;
    // }
}
void SheetAnimationComponent::onMessage(int id, const void* msg) {
    playAnimation((const char*)msg);
}
// void SheetAnimationComponent::setMountingPointData(const char* animName, MountingPoint data) {
//     m_mountingPointData.emplace(animName, data);
// }