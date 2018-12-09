#include <memory>
#include <random>
#include <vector>

template<typename T>
class Graph
{
public:
    void SetVerticesCount(const size_t verticesCount) {
        m_edgeTable.resize(verticesCount * verticesCount);
    }

private:
    std::vector<bool> m_edgeTable;

};

int main() {
    using Value = size_t;

    std::random_device randomDevice;
    std::mt19937_64 randomGenerator(randomDevice());
    std::uniform_int_distribution<Value> valueDistribution(std::numeric_limits<Value>::lowest(), std::numeric_limits<Value>::max());

    const size_t valuesCount = 10000;

    std::vector<Value> values;
    GenerateRandomVector(values, valuesCount, false, false, randomGenerator, valueDistribution);

    for (size_t i = 0; i < values.size(); ++i) {

    }

    return 0;
}
