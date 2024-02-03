#pragma once

#define MIN_TO_MS 60000

#ifndef ARDUINO
template <typename T>
T constrain(T value, T low, T high) {
    if (value < low)
 {
    if (value < low)        return low;
    if (value > high)
        return high;
    return value;
}
#endif

float mapf(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

float mapConstrainf(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return constrain(mapf(value, fromLow, fromHigh, toLow, toHigh), toLow, toHigh);
}

template <typename T>
T wrap(T value, T low, T high) {
    T range = high - low;
    while (value < low)
        value += range;
    while (value > high)
        value -= range;
    return value;
}

