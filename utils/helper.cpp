#include "helper.h"
#include <string.h>

void wavHeader(uint8_t* header, int wavSize)
{
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSize = wavSize + WaveHeaderSize - 8;
    header[4] = (uint8_t)(fileSize & 0xFF);
    header[5] = (uint8_t)((fileSize >> 8) & 0xFF);
    header[6] = (uint8_t)((fileSize >> 16) & 0xFF);
    header[7] = (uint8_t)((fileSize >> 24) & 0xFF);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 0x10;
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01;
    header[21] = 0x00;
    header[22] = 0x01;
    header[23] = 0x00;
    header[24] = 0x80;
    header[25] = 0x3E;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x00;
    header[29] = 0x7D;
    header[30] = 0x00;
    header[31] = 0x00;
    header[32] = 0x02;
    header[33] = 0x00;
    header[34] = 0x10;
    header[35] = 0x00;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (uint8_t)(wavSize & 0xFF);
    header[41] = (uint8_t)((wavSize >> 8) & 0xFF);
    header[42] = (uint8_t)((wavSize >> 16) & 0xFF);
    header[43] = (uint8_t)((wavSize >> 24) & 0xFF);
}

uint32_t timeNow(int timeZone) {
    time_t now;
    time(&now);
    return (uint32_t)now + timeZone * 3600;
}
void trim(char *s) {
    //fb   trim in place
    char *pe;
    char *p = s;
    while ( isspace(*p) ) p++; //left
    pe = p; //right
    while ( *pe != '\0' ) pe++;
    do {
        pe--;
    } while ( (pe > p) && isspace(*pe) );
    if (p == s) {
        *++pe = '\0';
    } else {  //move
        while ( p <= pe ) *s++ = *p++;
        *s = '\0';
    }
}

bool startsWith (const char* base, const char* str) {
//fb
    char c;
    while ( (c = *str++) != '\0' )
        if (c != *base++) return false;
    return true;
}

bool endsWith (const char* base, const char* str) {
//fb
    int slen = strlen(str) - 1;
    const char *p = base + strlen(base) - 1;
    while(p > base && isspace(*p)) p--;  // rtrim
    p -= slen;
    if (p < base) return false;
    return (strncmp(p, str, slen) == 0);
}

int indexOf (const char* base, const char* str, int startIndex) {
//fb
    const char *p = base;
    for (; startIndex > 0; startIndex--)
        if (*p++ == '\0') return -1;
    char* pos = strstr(p, str);
    if (pos == nullptr) return -1;
    return pos - base;
}

int indexOf (const char* base, char ch, int startIndex) {
//fb
    const char *p = base;
    for (; startIndex > 0; startIndex--)
        if (*p++ == '\0') return -1;
    char *pos = strchr(p, ch);
    if (pos == nullptr) return -1;
    return pos - base;
}

int lastIndexOf(const char* haystack, const char* needle) {
//fb
    int nlen = strlen(needle);
    if (nlen == 0) return -1;
    const char *p = haystack - nlen + strlen(haystack);
    while (p >= haystack) {
        int i = 0;
        while (needle[i] == p[i])
        if (++i == nlen) return p - haystack;
        p--;
    }
    return -1;
}

int lastIndexOf(const char* haystack, const char needle) {
//fb
    const char *p = strrchr(haystack, needle);
    return (p ? p - haystack : -1);
}

int specialIndexOf (uint8_t* base, const char* str, int baselen, bool exact){
    int result;  // seek for str in buffer or in header up to baselen, not nullterninated
    if (strlen(str) > baselen) return -1; // if exact == true seekstr in buffer must have "\0" at the end
    for (int i = 0; i < baselen - strlen(str); i++){
        result = i;
        for (int j = 0; j < strlen(str) + exact; j++){
            if (*(base + i + j) != *(str + j)){
                result = -1;
                break;
            }
        }
        if (result >= 0) break;
    }
    return result;
}
size_t bigEndian(uint8_t* base, uint8_t numBytes, uint8_t shiftLeft){
    size_t result = 0;
    if(numBytes < 1 or numBytes > 4) return 0;
    for (int i = 0; i < numBytes; i++) {
            result += *(base + i) << (numBytes -i - 1) * shiftLeft;
    }
    return result;
}

size_t urlencode_expected_len(const char* source){
    size_t expectedLen = strlen(source);
    for(int i = 0; i < strlen(source); i++) {
        if(isalnum(source[i])){;}
        else expectedLen += 2;
    }
    return expectedLen;
}