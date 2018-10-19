#include <cassert>
#include <iterator>
#include <string>
#include <sstream>

size_t GetDigitsCount(size_t number) {
    size_t count = 0;

    while (number > 0) {
        ++count;
        number /= 10;
    }

    return count;
}

template<bool skipSmallSequences = true>
std::string CompressString(const std::string_view str) {
    std::stringstream stream;

    char currentChar = 0;
    size_t count = 0;

    auto flushAndCompress = [&stream](char chr, size_t n) {
        if (n == 0)
            return;

        if constexpr (skipSmallSequences) {
            const size_t compressedSymbolsCount = GetDigitsCount(n) + 3;
            if (compressedSymbolsCount > n) {
                for (size_t i = 0; i < n; ++i) {
                    stream << chr;
                }
                return;
            }
        }

        stream << chr << '(' << n << ')';
    };

    for (char c : str) {
        if (c != currentChar) {
            flushAndCompress(currentChar, count);
            currentChar = c;
            count = 1;
        } else {
            ++count;
        }
    }

    flushAndCompress(currentChar, count);

    return stream.str();
}

int main() {
    {
        const std::string_view arg = "11111111112";
        const std::string_view expected = "1(10)2";
        auto result = CompressString(arg);
        assert(result == expected);
        assert(result.size() <= arg.size());
    }

    {
        const std::string_view arg = "11111222222";
        const std::string_view expected = "1(5)2(6)";
        auto result = CompressString(arg);
        assert(result == expected);
        assert(result.size() <= arg.size());
    }

    return 0;
}
