#include "text_label.h"
#include "utils/helper.h"

TextLabel::TextLabel(uint16_t width, uint16_t height, uint16_t color,const char* text, bool adaptiveSize)
: Widget(width, height), m_Color(color), m_bAdaptiveSize(adaptiveSize) {
    setText(text);
}   

void TextLabel::setText(const char* text) {
    if (m_Text == text)
        return;
    m_Text = text;
    clearDrawables();
    if (text == nullptr || strlen(text) == 0) return;
    if (m_bAdaptiveSize) {
        auto rect = BMFont::calculateUTFStringRect(text);
        m_width = rect.x;
        m_height = rect.y;
    }
    auto drawable = BMFont::create(Vector2(m_width, m_height), text, m_Color);
    attachDrawable(drawable);
    align();
}

void TextLabel::setColor(uint16_t color) {
    if (color == m_Color) return;
    auto& drawables = getDrawables();
    if (!drawables.empty()) {
        auto ptr = drawables.at(0);
        ((BMFont*)&*ptr)->setColor(color);
    }
    m_Color = color;
}