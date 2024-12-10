#include <cmath>
#include "sheet_animation_component.h"
#include "node.h"
#include "math/vector3.h"


SheetAnimationComponent::SheetAnimationComponent()
: m_pSheet(nullptr),m_bLoop(true),m_bRevert(false),m_fSpeed(1.0f),m_fAnimElapse(0),m_pCurrentAnimData(nullptr)
{
    m_currentMountingPoint.len = 0;
    m_currentMountingPoint.data = nullptr;
}
void SheetAnimationComponent::onAttachTarget(Node* target) {
    Component::onAttachTarget(target);
    registerLocalMessage(target, Msg_PlayAnim);
}
void SheetAnimationComponent::update(Node* target, float deltaTime) {
    if (!m_pSheet) {
        auto& drawables = target->getDrawables();
        for (Drawable* d : drawables) {
            m_pSheet = d->cast<SpriteSheet>();
            if (m_pSheet)
                break;
        }
    }
    if (m_pCurrentAnimData && m_pSheet) {
        int count = m_pCurrentAnimData->indices.size();
        int index = (int)(m_fAnimElapse / m_pCurrentAnimData->interval);
        if (m_pCurrentAnimData->loop)
            index = index % count;
        if (index < 0) {
            index = count - index;
        }
        if (index >= count)
            return;
        if (index < m_currentMountingPoint.len) {
            int offset = index * 3;
            auto& pivot = m_pSheet->getPivot();
            int xoffset = m_pSheet->getSize().x * pivot.x;
            int yoffset = m_pSheet->getSize().y * pivot.y;
            Vector3h mpt(m_currentMountingPoint.data[offset] - xoffset,
             m_currentMountingPoint.data[offset+1] - yoffset, m_currentMountingPoint.data[offset+2]);
            target->broadcastMessage(Msg_MountingPointPos, &mpt);
        }
        uint8_t frameIndex = m_pCurrentAnimData->indices.at(index);
        auto itr = m_extraAnimDataLookup.find(m_currentAnim);
        if (itr != m_extraAnimDataLookup.end()) {
            uint16_t w = itr->second.width / itr->second.col;
            uint16_t h = itr->second.height / itr->second.row;
            uint32_t offset = frameIndex*w*h;
            if (itr->second.palette)
                offset /= (16 / itr->second.bpp);
            m_pSheet->setFrameData(itr->second.data + offset, w, h, itr->second.maskColor);
        } else {
            m_pSheet->setFrame(frameIndex);
        }
    }
    m_fAnimElapse += deltaTime * m_fSpeed;
}
void SheetAnimationComponent::setAnimation(const char*  animName, std::vector<unsigned char> indices,float interval, bool loop) {
    AnimData data = AnimData();
    data.interval = interval;
    data.indices = indices;
    data.loop = loop;
    m_Anims.emplace(animName, data);
}
void SheetAnimationComponent::setExtraAnimation(const ImageData& imgData, const char*  animName, std::vector<unsigned char> indices,float interval, bool loop) {
    auto itr = m_extraAnimDataLookup.emplace(animName, imgData);
    if (!itr.second) {
        return;
    }
    AnimData data = AnimData();
    data.interval = interval;
    data.indices = indices;
    data.loop = loop;
    m_extraAnims.emplace(animName, data);
}

void SheetAnimationComponent::playAnimation(const char* animName, float speed) {
    auto itr = m_Anims.find(animName);
    // looking for sheet sprite's animation first
    if (itr != m_Anims.end()) {
        m_pCurrentAnimData = &itr->second;
        if (m_currentAnim == animName && m_pCurrentAnimData->loop) {
            return;
        }
        m_currentAnim = animName;
        m_fSpeed = speed;
        m_fAnimElapse = 0;
    } else { // then looking for extra animations
        itr = m_extraAnims.find(animName);
        if (itr != m_extraAnims.end()) {
            m_currentAnim = animName;
            m_pCurrentAnimData = &itr->second;
            m_fSpeed = speed;
            m_fAnimElapse = 0;
        } else {
            printf("animation not found : %s\n", animName);
        }
    }
    auto mptItr = m_mountingPointData.find(animName);
    if (mptItr != m_mountingPointData.end()) {
        m_currentMountingPoint = mptItr->second;
    }
}
void SheetAnimationComponent::onMessage(int id, const void* msg) {
    playAnimation((const char*)msg);
}
void SheetAnimationComponent::setMountingPointData(const char* animName, MountingPoint data) {
    m_mountingPointData.emplace(animName, data);
}