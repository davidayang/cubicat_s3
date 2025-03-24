#ifndef _TEXT_LABEL_H_
#define _TEXT_LABEL_H_
#include "widget.h"
#include "graphic_engine/drawable/bmfont.h"

namespace cubicat {

class TextLabel : public Widget {
public:
    DECLARE_RTTI_SUB(TextLabel, Widget);
    static SharedPtr<TextLabel> create(uint16_t width, uint16_t height, uint16_t color, const char* text,
     bool adaptiveSize = false) {
        return SharedPtr<TextLabel>(NEW TextLabel(width, height, color, text, adaptiveSize));
    }

    void setText(const char* text);
    const std::string& getText(){ return m_Text; }
private:
    TextLabel(uint16_t width, uint16_t height, uint16_t color,const char* text, bool adaptiveSize);
    uint16_t        m_Color;
    bool            m_bAdaptiveSize;
    std::string          m_Text;
};
typedef SharedPtr<TextLabel> TextLabelPtr;
}
#endif