#ifndef _SHEETANIMATIONCOMPONENT_H_
#define _SHEETANIMATIONCOMPONENT_H_
#include "component.h"
#include <string.h>
#include <vector>
#include <map>
#include "core/message/message_tube.h"
#include "../drawable/image_data.h"
#include "../mounting_point.h"
#include "../drawable/texture.h"
#include "../timer.h"

namespace cubicat {
struct SheetAnimData {
    std::vector<unsigned char>  indices;
    float                       interval;
};
typedef std::map<std::string,SheetAnimData> AnimMap;
class SheetAnimationComponent : public Component, protected MessageReceiver
{
public:
    static SharedPtr<SheetAnimationComponent> create() {
        return SharedPtr<SheetAnimationComponent>(NEW SheetAnimationComponent());
    }
    void update(Node* target, float deltaTime) override;
    void onAttachTarget(Node* target) override;
    void onMessage(int id, const void* msg);

    void setAnimation(const char* animName, std::vector<uint8_t> indices, float interval, bool loop=false);
    // void setMountingPointData(const char* animName, MountingPoint data);
    void playAnimation(const char* animName, float speed = 1.0f);
private:
    SheetAnimationComponent();
    Timer           m_timer;
    AnimMap         m_Anims;
    std::string     m_currentAnim;
    SheetAnimData*  m_pCurrentAnimData = nullptr;
    MountingPoint   m_currentMountingPoint;
    float           m_fSpeed = 1.0f;
    // std::map<std::string, MountingPoint>    m_mountingPointData;
};
typedef SharedPtr<SheetAnimationComponent> SheetAnimationComponentPtr;
}
#endif