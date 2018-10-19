#include <iostream>
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

std::string CompressString(const std::string_view str) {
    std::stringstream stream;

    auto flushCompressed = [&stream](char chr, size_t n) {
        if (n > 0) {
            if (GetDigitsCount(n) + 3 > n) {
                for (size_t i = 0; i < n; ++i) {
                    stream << chr;
                }
            } else {
                stream << chr << '(' << n << ')';
            }
        }
    };

    char currentChar = 0;
    size_t count = 0;

    for (char c : str) {
        if (c != currentChar) {
            flushCompressed(currentChar, count);
            currentChar = c;
            count = 1;
        } else {
            ++count;
        }
    }

    flushCompressed(currentChar, count);

    return stream.str();
}

void TestCase(std::ostream& output, std::string_view string, std::string_view expected) {
    const auto actual = CompressString(string);
    const char* testState = actual == expected ? "OK: " : "FAILED: ";

    output << testState << '\'' << string.data() << '\'' << " -> " << actual;
    output << '\t' << '\'' << expected.data() << '\'' << " expected" << '\n';
}

int main() {
    auto& output = std::cout;
    TestCase(output, "", "");
    TestCase(output, "1", "1");
    TestCase(output, "12", "12");
    TestCase(output, "222222", "2(6)");
    TestCase(output, "1222222", "12(6)");
    TestCase(output, "2222221", "2(6)1");
    TestCase(output, "11111222222", "1(5)2(6)");
    TestCase(output, "11111233333", "1(5)23(5)");
    return 0;
}
