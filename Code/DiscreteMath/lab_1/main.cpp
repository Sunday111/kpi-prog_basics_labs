#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

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

template<typename T>
class AdajencyMatrix
{
public:
    AdajencyMatrix(size_t verticesCount) :
        m_verticesCount(verticesCount),
        m_edgeFlags(EdgesCountByVerticesCount(verticesCount), false)
	{
		Assert(verticesCount > 0, "vertices count > 2");
    }

    void SetEdge(size_t i, size_t j, T value) {
		ValidateIndices(i, j);
        const size_t idx = EdgeIndex(i, j);
        m_edgeFlags[idx] = value;
    }

	const T& GetEdge(size_t i, size_t j) const {
		ValidateIndices(i, j);
        const size_t idx = EdgeIndex(i, j);
        return m_edgeFlags[idx];
    }

private:
	template<typename... T>
	inline void ValidateIndices(T... indices) const {
		(ValidateIndex(indices), ...);
	}

	inline void ValidateIndex(size_t index) const {
		Assert(index < m_verticesCount, "index < vertices count");
	}

    [[nodiscard]] static inline constexpr size_t EdgesCountByVerticesCount(const size_t verticesCount) noexcept {
        return verticesCount * (verticesCount - 1) / 2;
    }

    [[nodiscard]] static inline constexpr std::pair<size_t, size_t> CorrectIndices(const size_t i, const size_t j) noexcept {
        return i > j ? std::pair (j, i) : std::pair(i, j);
    }

    [[nodiscard]] static inline constexpr size_t EdgeIndex(const size_t i_, const size_t j_) noexcept {
        const auto[i, j] = CorrectIndices(i_, j_);
        return EdgesCountByVerticesCount(j) + i;
    }

private:
    size_t m_verticesCount;
    std::vector<T> m_edgeFlags;
};

int main() {
    std::istream& input = std::cin;
    std::ostream& output = std::cout;

    try
    {
        const int verticesCount = MustReadValue<int>(input, "Read vertices count");


		AdajencyMatrix<bool> adajencyMatrix(verticesCount);
		adajencyMatrix.SetEdge(0, 1, true);
		bool val = adajencyMatrix.GetEdge(1, 0);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    

    return 0;
}