constexpr unsigned GetStringSpacesCount(const char* str) {
    unsigned result = 0;

    if (str) {
        for (unsigned i = 0; str[i] != '\0'; ++i) {
            if (str[i] == ' ') {
                ++result;
            }
        }
    }

    return result;
}

int main(int, char**) {
    static_assert(GetStringSpacesCount("123") == 0);
    static_assert(GetStringSpacesCount("1 2 3 ") == 3);
    static_assert(GetStringSpacesCount(" ") == 1);
    return 0;
}
