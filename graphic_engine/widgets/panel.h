// #ifndef _PANEL_H_
// #define _PANEL_H_
// #include "widget.h"   

// class Panel : public Widget {
// public:
//     DECLARE_RTTI_SUB(Panel, Widget);
//     static SharedPtr<Panel> create(uint16_t width, uint16_t height, uint16_t color, bool fill, uint16_t cornerRadius = 0, uint8_t lineWidth = 1) {
//         return SharedPtr<Panel>(NEW Panel(width, height, color, fill, cornerRadius, lineWidth));
//     }
//     void setColor(uint16_t color);
// private:
//     Panel(uint16_t width, uint16_t height, uint16_t color, bool fill, uint16_t cornerRadius, uint8_t lineWidth);
// };
// typedef SharedPtr<Panel> PanelPtr;
// #endif