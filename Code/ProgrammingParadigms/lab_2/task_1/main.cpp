#include <cmath>
#include <iostream>

template<typename T>
T taskFunction(const T t, const size_t i) {
    if (i > 2) {
        T result{};

        for (size_t k = 1; k <= i; ++k) {
            result += 1 / k;
        }

        return result * std::sin(t);
    }
    return std::log(t);
}

template<typename T>
T AskUserForParameter(const char* name, std::ostream& output, std::istream& input) {
    T result{};
    output << name << " = ";
    input >> result;
    return result;
}

int main() {
    auto& output = std::cout;
    auto& input = std::cin;

    auto t = AskUserForParameter<float>("t", output, input);
    auto i = AskUserForParameter<int>("i", output, input);

    if (i < 1) {
        output << "Invalid input parameter i. It should be more than zero" << std::endl;
        return 1;
    }

    auto result = taskFunction(t, static_cast<size_t>(i));
    output << "Result: " << result << std::endl;

    return 0;
}
