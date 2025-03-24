#include "widget.h"

using namespace cubicat;
Widget::Widget(uint16_t width, uint16_t height)
 : m_width(width), m_height(height), m_eAlignment(Alignment::NONE) {
}

Widget::~Widget() {
}

void Widget::align() {
    if (m_eAlignment == NONE || !getParent() || !getParent()->ofA<Widget>()) {
        return;
    }
    Widget* parent = getParent()->cast<Widget>();
    if (!parent) {
        return;
    }
    if (m_eAlignment == LEFT) {
        setPosition(0, getPosition().y);
    } else if (m_eAlignment == RIGHT) {
        setPosition(parent->getWidth() - m_width, getPosition().y);
    } else if (m_eAlignment == TOP) {
        setPosition(getPosition().x, parent->getHeight() - m_height);
    } else if (m_eAlignment == BOTTOM) {
        setPosition(getPosition().x, 0);
    } else if (m_eAlignment == CENTER) {
        setPosition((parent->getWidth() - m_width) / 2, (parent->getHeight() - m_height) / 2);
    } else if (m_eAlignment == LEFTTOP) {
        setPosition(0, parent->getHeight() - m_height);
    } else if (m_eAlignment == LEFTCENTER) {
        setPosition(0, (parent->getHeight() - m_height) / 2);
    } else if (m_eAlignment == LEFTBOTTOM) {
        setPosition(0, 0);
    } else if (m_eAlignment == TOPCENTER) {
        setPosition((parent->getWidth() - m_width) / 2, parent->getHeight() - m_height);
    } else if (m_eAlignment == BOTTOMCENTER) {
        setPosition((parent->getWidth() - m_width) / 2, 0);
    } else if (m_eAlignment == RIGHTTOP) {
        setPosition(parent->getWidth() - m_width, parent->getHeight() - m_height);
    } else if (m_eAlignment == RIGHTCENTER) {
        setPosition(parent->getWidth() - m_width, (parent->getHeight() - m_height) / 2);
    } else if (m_eAlignment == RIGHTBOTTOM) {
        setPosition(parent->getWidth() - m_width, 0);
    }

}
void Widget::setAlignment(Alignment alignment) {
    m_eAlignment = alignment;
    onSetParent();
}