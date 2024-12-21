#ifndef _WIDGET_H_
#define _WIDGET_H_
#include "../node.h"


class Widget : public Node {
public:
    DECLARE_RTTI_SUB(Widget, Node);
    enum Alignment {
        NONE = -1,
        LEFT = 0,
        RIGHT = 1,
        TOP = 2,
        BOTTOM = 3,
        CENTER = 4,
        LEFTTOP = 5,
        LEFTCENTER = 6,
        LEFTBOTTOM = 7,
        TOPCENTER = 8,
        BOTTOMCENTER = 9,
        RIGHTTOP = 10,
        RIGHTCENTER = 11, 
        RIGHTBOTTOM = 12
    };
    static SharedPtr<Widget> create(uint16_t width, uint16_t height) {
        return SharedPtr<Widget>(NEW Widget(width, height));
    }
    virtual ~Widget();
    void setAlignment(Alignment alignment);
    Alignment getAlignment() { return m_eAlignment; }
    uint16_t getWidth() { return m_width; }
    uint16_t getHeight() { return m_height; }
protected:
    Widget(uint16_t width, uint16_t height);
    Widget() = delete;
    void onSetParent() override { align(); }
    void align();
    uint16_t        m_width = 0;
    uint16_t        m_height = 0;
    Alignment       m_eAlignment = Alignment::NONE;
};

typedef SharedPtr<Widget> WidgetPtr;

#endif