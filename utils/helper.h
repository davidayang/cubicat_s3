#ifndef _HELPER_H_
#define _HELPER_H_
#include <string>
#include <vector>
#include <unordered_map>
#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

static int64_t _startTime, _endTime;
static std::unordered_map<std::string, uint32_t> _classifiedStatistics;
struct TimeStack {
    std::string name;
    int64_t    time;  
};
static TimeStack _timerStack[32]; static int _timerStackDepth = 0;


#define BENCHMARK(code)    \
    _startTime = micros(); \
    code;                  \
    _endTime = micros();   \
    printf("file:%s line[%d]: %ld ms (%ld us)\n", __FILE__, __LINE__, uint32_t(_endTime - _startTime)/1000, uint32_t(_endTime - _startTime));

#define BENCHMARK_CLASS_START(cls)  \
    _startTime = micros();          \
    assert(_timerStackDepth < 32);  \
    for (int i = _timerStackDepth-1; i >= 0; i--) { \
        if (_timerStack[i].name == cls) { \
            for (int j = i; j < _timerStackDepth; j++) \
                _classifiedStatistics[_timerStack[j].name] += _startTime - _timerStack[j].time; \
            _timerStackDepth = i; \
            break; \
        } \
    } \
    _timerStack[_timerStackDepth++] = {cls, _startTime};


#define BENCHMARK_CLASS_END(cls)    \
    _endTime = micros();   \
    _classifiedStatistics[cls] += _endTime - _timerStack[--_timerStackDepth].time;

#define BENCHMARK_CLASS_RESET \
    _classifiedStatistics.clear(); \
    _timerStackDepth = 0;

#define BENCHMARK_CLASS_REPORT \
    for (auto it = _classifiedStatistics.begin(); it != _classifiedStatistics.end(); it++) \
        printf("%s: %ld ms (%ld us)\n", it->first.c_str(), uint32_t(it->second)/1000, uint32_t(it->second));


#define MEMORY_REPORT memoryReport(__FILE__, __LINE__);

const int WaveHeaderSize = 44;
extern void wavHeader(uint8_t* header, int wavSize);

inline int getMainCoreId() {
#ifdef CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0
    return 0;
#else
    return 1;
#endif
}
#ifndef portNUM_PROCESSORS
#define portNUM_PROCESSORS 1
#endif

inline int getSubCoreId() {
    return (getMainCoreId() + 1) % portNUM_PROCESSORS;
}
inline std::vector<std::string> splitString(const std::string& str, const char* delimiter) {
    std::vector<std::string> result;
    std::size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start));
    return result;
}

inline time_t timeStringToTimestamp(const std::string& timeString) {
    std::tm tm = {};
    std::istringstream ss(timeString);
    ss >> std::get_time(&tm, "%y:%m:%d %H:%M:%S");
    if (ss.fail()) {
        return 0;
    }
    // 将 tm 转换为 time_t（Unix 时间戳）
    return std::mktime(&tm);
}
inline std::string timestampToTimeString(time_t timestamp) {
    std::tm tm = *std::localtime(&timestamp);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%y:%m:%d %H:%M:%S");
    return ss.str();
}

void trim(char *s);

bool startsWith (const char* base, const char* str);

bool endsWith (const char* base, const char* str);

int indexOf (const char* base, const char* str, int startIndex);

int indexOf (const char* base, char ch, int startIndex);

int lastIndexOf(const char* haystack, const char* needle);

int lastIndexOf(const char* haystack, const char needle);

void toLowerCase(char* str);

int specialIndexOf (uint8_t* base, const char* str, int baselen, bool exact = false);

size_t bigEndian(uint8_t* base, uint8_t numBytes, uint8_t shiftLeft = 8) ;

size_t urlencode_expected_len(const char* source);

size_t base64LenExpected(size_t bufLen);

bool base64Encode();

// [JS_BINDING_BEGIN]
uint32_t timeNow(int timeZone = 8);
// [JS_BINDING_END]
extern "C" void memoryReport(const char* file, int line);

extern "C" uint32_t millis();
extern "C" int64_t micros();

#endif