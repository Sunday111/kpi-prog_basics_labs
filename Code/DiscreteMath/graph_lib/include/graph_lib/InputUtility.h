#pragma once

#include "ErrorHandling.h"

template<typename T>
inline bool TryReadValue(std::istream& input, T& value) {
    return !(!(input >> value));
}

template<typename T>
inline void MustReadValue(std::istream& input, T& value, const std::string_view message = MustReadValueMessage) {
    Assert(TryReadValue(input, value), message);
}

template<typename T>
inline T MustReadValue(std::istream& input, const std::string_view message = MustReadValueMessage) {
    T result;
    MustReadValue(input, result, message);
    return result;
}
