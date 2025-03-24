#include "drawable.h"

using namespace cubicat;


Drawable::Drawable() {
    m_pMaterial = Material::create();
    m_pMaterial->setTexture(SharedPtr<Texture>(NEW SolidTexture()));
}

Vector2f Drawable::getScale() {
    return {m_scale.x, m_scale.y};
}

Vector2f Drawable::getPosition() {
    return {m_pos.x, m_pos.y};
}

void Drawable::update(const Vector2f& pos,const Vector2f& scale, int16_t angle) {
    m_scale.x = scale.x;
    m_scale.y = scale.y;
    m_pos.x = pos.x;
    m_pos.y = pos.y;
    m_angle = angle % 360;
    if (m_angle < 0)
        m_angle = 360 + m_angle;
    updateRegion();
}

void Drawable::updateRegion() {
    auto tex = m_pMaterial->getTexture();
    bool hasScale = abs(m_scale.x - 1) > 0.0001 || abs(m_scale.y - 1) > 0.0001;
    bool hasRot = m_angle % 360 != 0;
    auto texSize = tex->getTextureSize();
    int16_t p1x = -texSize.x * m_pivot.x; //图片局部坐标系
    int16_t p1y = texSize.y * (1 - m_pivot.y);
    if (hasScale) {
        p1x *= abs(m_scale.x);
        p1y *= abs(m_scale.y);
        texSize.x = ceil(texSize.x * abs(m_scale.x));
        texSize.y = ceil(texSize.y * abs(m_scale.y));
    }
    int16_t ltx = p1x; int16_t lty = p1y;
    if (hasRot) {
        int16_t sina = getSinValue(m_angle);
        int16_t cosa = getCosValue(m_angle);
        int16_t p2x = p1x + texSize.x;
        int16_t p2y = p1y;
        int16_t p3x = p2x;
        int16_t p3y = p2y - texSize.y;
        int16_t p4x = p1x;
        int16_t p4y = p3y;
        RotatePoint(p1x, p1y, sina, cosa)
        RotatePoint(p2x, p2y, sina, cosa)
        RotatePoint(p3x, p3y, sina, cosa)
        RotatePoint(p4x, p4y, sina, cosa)
        // calculate AABB
        int16_t xmin = std::min(p1x, std::min(p2x, std::min(p3x, p4x)));
        int16_t xmax = std::max(p1x, std::max(p2x, std::max(p3x, p4x)));
        int16_t ymin = std::min(p1y, std::min(p2y, std::min(p3y, p4y)));
        int16_t ymax = std::max(p1y, std::max(p2y, std::max(p3y, p4y)));
        ltx = xmin;
        lty = ymax;
        texSize.x = xmax - xmin;
        texSize.y = ymax - ymin;
    }
    int16_t x = m_pos.x + ltx;
    int16_t y = m_pos.y + lty;
    m_region.x = x;
    m_region.y = y;
    m_region.w = texSize.x;
    m_region.h = texSize.y;
}

DrawableMaterial Drawable::getMaterial() {
    return DrawableMaterial(m_pMaterial, this);
}
