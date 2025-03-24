#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdint.h>

namespace cubicat {

#define FP_SCALE_SHIFT  10
#define ANGLE_2_RAD(a)   a * 0.0174533
#define RAD_2_ANGLE(r)   r * 57.295780
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

constexpr uint16_t FP_SCALE = (1 << FP_SCALE_SHIFT);

// sin cos value lookup in degrees[0-360)
int16_t getSinValue(uint16_t angle);
int16_t getCosValue(uint16_t angle);

// faster than std::sort in small array
template <typename T>
void insertSortAsc(T* array, int n) {
    for (int i = 1; i < n; i++) {
        const T& key = array[i];
        int j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

template <typename T>
void insertSortDesc(T* array, int n) {
    for (int i = 1; i < n; i++) {
        const T& key = array[i];
        int j = i - 1;
        while (j >= 0 && array[j] < key) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

}
#endif