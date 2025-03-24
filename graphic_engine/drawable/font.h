#ifndef _FONT_H_
#define _FONT_H_
#include <stdint.h>
#include <vector>
#include <string.h>
#include <string>

struct FontData {
    const uint8_t* glyphData;
    uint8_t fontSize;
    uint16_t charCount;
    const char* charSet;
};
extern FontData DefaultFontData;

int getCharIndex(const char* str, const char* target);
std::vector<std::string> splitUTF8(const char* s);
void calculateUTFStringSize(const char* text, uint16_t* width, uint16_t* height);

#endif
