#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_
#include <cstdint>
#include "core/shared_pointer.h"
#include "../math/vector2.h"
#include "core/rtti.h"
#include "core/memory_object.h"
#include "core/id_object.h"
#include "../region.h"
#include "../math/common.h"
#include "mesh/mesh.h"
#include "../renderer/render_state.h"
#include "material.h"

namespace cubicat {

class Drawable;
typedef DirtyNotifyPointer<Material,Drawable> DrawableMaterial;

class Drawable : public RTTI, public MemoryObject, public IDObject, public Dirtyable
{
public:
    DECLARE_RTTI_ROOT(Drawable);
    static SharedPtr<Drawable> create() {return SharedPtr<Drawable>(new Drawable());}
    static bool zCompareAsc(const SharedPtr<Drawable>& a, const SharedPtr<Drawable>& b) {return a->getZOrder() < b->getZOrder();}
    static bool zCompareDesc(const SharedPtr<Drawable>& a, const SharedPtr<Drawable>& b) {return a->getZOrder() > b->getZOrder();}
    virtual ~Drawable() {}
    void update(const Vector2f& pos,const Vector2f& scale, int16_t rotation);
    virtual void setPivot(float x, float y) {m_pivot.x = x; m_pivot.y = y; addDirty(true);}
    virtual void onFinishDraw() {}

    Vector2f getScale();
    Vector2f getPosition();
    int16_t getAngle() {return m_angle;} 
    const Vector2f& getPivot() {return m_pivot;} 
    // bounding box in world space, y up
    Region getRegion() {return m_region;}
    void setVisible(bool visible) {m_bVisible = visible;}
    bool isVisible() {return m_bVisible;}
    void setMaterial(MaterialPtr m) { m_pMaterial = m;}
    DrawableMaterial getMaterial();
    float getZOrder() const { return m_fZOrder; }
    // [JS_BINDING_BEGIN]
    MaterialPtr materialPtr() { return m_pMaterial; }
    // [JS_BINDING_END]
protected:
    Drawable();
    virtual void updateRegion();

    MaterialPtr         m_pMaterial;
    // boundingbox in 2d world space (Y up)
    Region              m_region;
    Vector3f            m_scale;
    Vector3f            m_pos;
    float               m_fZOrder = 0.0f;
private:
    int16_t             m_angle;
    bool                m_bVisible = true;
    Vector2f            m_pivot = Vector2f(0.5f, 0.5f);
};

typedef SharedPtr<Drawable>       DrawablePtr;

inline uint16_t* generateSinglePalette(uint16_t color) {
    uint16_t maskColor = 0xFFFF - color;
    uint16_t* palette = new uint16_t[2];
    palette[0] = maskColor;
    palette[1] = color;
    return palette;
}
#define RotatePoint(x, y, sina, cosa) { \
    uint16_t x1 = (x * cosa - y * sina) >> FP_SCALE_SHIFT; \
    uint16_t y1 = (x * sina + y * cosa) >> FP_SCALE_SHIFT; \
    x = x1; \
    y = y1; } \

#define RotatePointFloat(x, y, sina, cosa) { \
    float x1 = (x * cosa - y * sina) / FP_SCALE; \
    float y1 = (x * sina + y * cosa) / FP_SCALE; \
    x = x1; \
    y = y1; } \

}

#endif