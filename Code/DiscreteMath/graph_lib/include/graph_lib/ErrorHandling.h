#pragma once

#include <cassert>
#include <stdexcept>
#include <sstream>
#include <string_view>

constexpr bool UseExceptions = true;

static constexpr std::string_view MustReadValueMessage = "Reading value from input stream";

inline void Assert(const bool value, const std::string_view message)
{
    if(!value)
    {
        if constexpr (UseExceptions)
        {
            static constexpr std::string_view AssertionFailedMessage = "Assertion failed: ";
            std::stringstream sstream;
            sstream << AssertionFailedMessage << message;
            throw std::runtime_error(sstream.str());
        }
        else
        {
            assert(false);
        }
    }
}
