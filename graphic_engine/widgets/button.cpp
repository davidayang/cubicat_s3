#include "button.h"

Button::Button(uint16_t width, uint16_t height, const char* text, OnConfirm onConfirm)
 : Widget(width, height), m_onConfirm(onConfirm) {

}
void Button::init(uint16_t textColor, uint16_t bgColor, const char* text, uint8_t cornerRadius) {
    m_pTextLabel = TextLabel::create(m_width, BMFont::getFontSize() * 1.5, textColor, text, true);
    m_pTextLabel->setAlignment(Widget::Alignment::CENTER);
    m_pBackground = Panel::create(m_width, m_height, bgColor, true, cornerRadius);
    m_pTextLabel->setParent(m_pBackground);
    m_pBackground->setParent(this);
}

void Button::confirm() {
    if (m_onConfirm) {
        m_onConfirm(nullptr);
    }
}
void Button::setText(const char* text, uint16_t textColor) {
    m_pTextLabel->setText(text);
    m_pTextLabel->setColor(textColor);
}
void Button::setBGColor(uint16_t bgColor) {
    m_pBackground->setColor(bgColor);
}
const std::string& Button::getText() {
    return m_pTextLabel->getText();
}