#include <string_view>

constexpr std::size_t GetStringSpacesCount(const std::string_view str) {
    size_t result = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == ' ') {
            ++result;
        }
    }
    return result;
}

int main(int, char**) {
    static_assert(GetStringSpacesCount(std::string_view("123")) == 0);
    static_assert(GetStringSpacesCount(std::string_view("1 2 3 ")) == 3);
    static_assert(GetStringSpacesCount(std::string_view(" ")) == 1);
    return 0;
}
