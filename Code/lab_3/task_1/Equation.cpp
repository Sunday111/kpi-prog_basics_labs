#include <cmath>
#include <iostream>
#include <string_view>

struct FunctionInput
{
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
};

float f7(const FunctionInput& i) noexcept {
    using namespace std;
    const float left = 4 * cosh(sqrt(abs(i.a / i.b)));
    const float right = 3 * acos(i.d);
    return pow(left + right, i.c);
}

FunctionInput GetUserInput(std::istream& input, std::ostream& output) {
    FunctionInput result{};

    auto readValueWithMessage = [&](const char* message, auto& value) {
        output << message;
        input >> value;
    };

    readValueWithMessage("Enter A: ", result.a);
    readValueWithMessage("Enter B: ", result.b);
    readValueWithMessage("Enter C: ", result.c);
    readValueWithMessage("Enter D: ", result.d);

    return result;
}

constexpr FunctionInput GetPredefinedUserInput() noexcept {
    return FunctionInput{ -3.45f, 2.34f, 1.45f, 0.83f };
}

template<typename T>
void PrintValue(std::ostream& output, const char* title, const T& value) {
    output << title << result << std::endl;
}

int main() {
    constexpr bool PredefinedInput = true;
    std::ostream& output = std::cout;
    std::istream& input = std::cin;

    const FunctionInput userInput =
        PredefinedInput ? (GetPredefinedUserInput()) : GetUserInput(input, output);
    const auto result = f7(userInput);
    PrintValue(output, "Result: ", result);

    return 0;
}
