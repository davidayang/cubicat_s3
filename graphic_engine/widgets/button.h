#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "text_label.h"
#include "panel.h"

typedef void(*OnConfirm)(void* param);


class Button : public Widget {
public:
    DECLARE_RTTI_SUB(Button, Widget);
    static SharedPtr<Button> create(uint16_t width, uint16_t height, uint16_t textColor, uint16_t bgColor, const char* text,
    OnConfirm onConfirm, uint8_t cornerRadius = 8) {
        auto ptr = SharedPtr<Button>(NEW Button(width, height, text, onConfirm));
        ptr->init(textColor, bgColor, text, cornerRadius);
        return ptr;
    }  
    void confirm();
    void cancel();
    void setText(const char* text, uint16_t textColor);
    void setBGColor(uint16_t bgColor);
    const std::string& getText();
private:
    Button(uint16_t width, uint16_t height, const char* text, OnConfirm onConfirm);
    void init(uint16_t textColor, uint16_t bgColor, const char* text, uint8_t cornerRadius);
    TextLabelPtr            m_pTextLabel;
    PanelPtr                m_pBackground;
    OnConfirm               m_onConfirm;
};

typedef SharedPtr<Button> ButtonPtr;
#endif