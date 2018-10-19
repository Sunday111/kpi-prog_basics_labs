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

constexpr FunctionInput GetPredefinedUserInput() {
    return FunctionInput{ -3.45f, 2.34f, 1.45f, 0.83f };
}

float f7(const FunctionInput& i) {
    const float left = 4 * std::cosh(std::sqrt(std::abs(i.a / i.b)));
    const float right = 3 * std::acos(i.d);
    return std::pow(left + right, i.c);
}

constexpr bool PredefinedInput = true;

int main() {
    using VariantFunction = float(*) (const FunctionInput&);
    constexpr VariantFunction variant = f7;

    std::ostream& output = std::cout;
    std::istream& input = std::cin;

    const FunctionInput userInput = PredefinedInput ? (GetPredefinedUserInput()) : GetUserInput(input, output);
    const auto result = variant(userInput);
    output << "Result: " << result << std::endl;

    return 0;
}