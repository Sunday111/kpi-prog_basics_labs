#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

enum class CopyMethod
{
    FilesystemCpp17,
    StdCopy,
    WriteRdBufToStream,
    AnsiC
};

template<CopyMethod method>
struct CopyImpl
{
};

template<>
struct CopyImpl<CopyMethod::FilesystemCpp17>
{
    static bool Call(const std::string_view& from, const std::string_view& to, std::ostream& logstream) {
        std::error_code errorCode;
        if (!std::filesystem::copy_file(from, to, errorCode)) {
            logstream << errorCode.message() << std::endl;
            return false;
        }

        return true;
    }
};

template<>
struct CopyImpl<CopyMethod::StdCopy>
{
    static bool Call(const std::string_view& from, const std::string_view& to, std::ostream& logstream) {
        std::ifstream source(from, std::ios::binary);
        if (!source.is_open()) {
            logstream << "Failed to open file: " << from << std::endl;
            return false;
        }

        std::ofstream destination(to, std::ios::binary);
        if (!destination.is_open()) {
            logstream << "Failed to open file: " << to << std::endl;
            return false;
        }

        std::copy(
            std::istreambuf_iterator<char>(source),
            std::istreambuf_iterator<char>(source),
            std::ostreambuf_iterator<char>(destination));

        return true;
    }
};

template<>
struct CopyImpl<CopyMethod::WriteRdBufToStream>
{
    static bool Call(const std::string_view& from, const std::string_view& to, std::ostream& logstream) {
        std::ifstream source(from, std::ios::binary);
        if (!source.is_open()) {
            logstream << "Failed to open file: " << from << std::endl;
            return false;
        }

        std::ofstream destination(to, std::ios::binary);
        if (!destination.is_open()) {
            logstream << "Failed to open file: " << to << std::endl;
            return false;
        }

        destination << source.rdbuf();

        return true;
    }
};

template<>
struct CopyImpl<CopyMethod::AnsiC>
{
    class AnsiFile
    {
    public:
        AnsiFile(const std::string_view& from, const std::string_view& mode) {
            m_pFile = fopen(from.data(), mode.data());
        }

        ~AnsiFile() {
            if (m_pFile) {
                fclose(m_pFile);
            }
        }

        bool IsOpen() const {
            return m_pFile != nullptr;
        }

        size_t Read(void* buffer, size_t elementSize, size_t elementCount) {
            if (!m_pFile) {
                return 0;
            }

            return fread(buffer, elementSize, elementCount, m_pFile);
        }

        size_t Write(void* buffer, size_t elementSize, size_t elementCount) {
            if (!m_pFile) {
                return 0;
            }

            return fwrite(buffer, elementSize, elementCount, m_pFile);
        }

    private:
        FILE* m_pFile = nullptr;
    };

    static bool Call(const std::string_view& from, const std::string_view& to, std::ostream& logstream) {
        AnsiFile src(from, "rb");

        if (!src.IsOpen()) {
            logstream << "Failed to open file: " << from << '\n';
            return false;
        }

        AnsiFile dst(to, "wb");

        if (!dst.IsOpen()) {
            logstream << "Failed to open file: " << to << '\n';
            return false;
        }

        constexpr size_t bufferSize = 8096;
        std::byte buffer[bufferSize]{};
        size_t readSize = 0;
        while (readSize = src.Read(buffer, 1, bufferSize)) {
            dst.Write(buffer, 1, bufferSize);
        }

        return true;
    }
};

template<CopyMethod method>
bool CopyFile(const std::string_view& from, const std::string_view& to, std::ostream& logstream) {
    return CopyImpl<method>::Call(from, to, logstream);
}

int main() {
    const std::string_view fileToCopy = "abc";
    const std::string_view copyName = "def";
    std::ostream& output = std::cout;
    constexpr CopyMethod copyMethod = CopyMethod::FilesystemCpp17;
    
    if (!CopyFile<copyMethod>(fileToCopy, copyName, output)) {
        return 1;
    }

    return 0;
}