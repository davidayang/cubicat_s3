#include "panel.h"
#include "drawable/rectangle.h"

Panel::Panel(uint16_t width, uint16_t height, uint16_t color, bool fill, uint16_t cornerRadius, uint8_t lineWidth)
: Widget(width, height) {
    auto drawable = Rectangle::create(width, height, color, fill, cornerRadius, lineWidth);
    attachDrawable(drawable);
}

void Panel::setColor(uint16_t color) {
    auto& drawables = getDrawables();
    if (!drawables.empty()) {
        auto ptr = drawables.at(0);
        ((Rectangle*)&*ptr)->setColor(color);
    }
}