#ifndef _SHEETANIMATIONCOMPONENT_H_
#define _SHEETANIMATIONCOMPONENT_H_
#include "component.h"
#include "drawable/sprite_sheet.h"
#include <string.h>
#include <vector>
#include <map>
#include "message/message_tube.h"
#include "drawable/image_data.h"
#include "mounting_point.h"

struct AnimData {
    std::vector<unsigned char>  indices;
    float                       interval;
    bool                        loop;
};
typedef std::map<std::string,AnimData> AnimMap;
class SheetAnimationComponent : public Component, protected MessageReceiver
{
public:
    static SharedPtr<SheetAnimationComponent> create() {
        return SharedPtr<SheetAnimationComponent>(NEW SheetAnimationComponent());
    }
    void update(Node* target, float deltaTime);
    void setAnimation(const char* animName, std::vector<uint8_t> indices, float interval, bool loop=false);
    void setExtraAnimation(const ImageData& imgData, const char*  animName, std::vector<uint8_t> indices, float interval, bool loop=false);
    void setMountingPointData(const char* animName, MountingPoint data);
    void onAttachTarget(Node* target) override;
    void playAnimation(const char* animName, float speed = 1.0f);
    void onMessage(int id, const void* msg);
private:
    SheetAnimationComponent();
    bool            m_bLoop;
    bool            m_bRevert;
    float           m_fAnimElapse;
    SpriteSheet*    m_pSheet;
    AnimMap         m_Anims;
    std::string     m_currentAnim;
    AnimData*       m_pCurrentAnimData;
    MountingPoint   m_currentMountingPoint;
    float           m_fSpeed;

    std::map<std::string, ImageData>        m_extraAnimDataLookup;
    AnimMap                                 m_extraAnims;
    std::map<std::string, MountingPoint>    m_mountingPointData;
};
typedef SharedPtr<SheetAnimationComponent> SheetAnimationComponentPtr;
#endif