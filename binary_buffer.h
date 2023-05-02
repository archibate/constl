#include <span>
#include <memory>
#include <vector>
#include <cstring>

struct BinaryReader {
private:
    std::span<std::byte const> buf;
    size_t offset = 0;

public:
    constexpr BinaryReader(std::span<std::byte const> buf) noexcept
    : buf(buf) {}

    template <class T>
    constexpr T read() {
        if (offset + sizeof(T) > buf.size())
            throw std::out_of_range("BinaryReader::read");
        alignas(alignof(T)) std::byte tmp[sizeof(T)];
        std::memcpy(tmp, buf.data() + offset, sizeof(T));
        offset += sizeof(T);
        return std::move(*reinterpret_cast<T *>(tmp));
    }
};

struct BinaryWriter {
    std::span<std::byte> buf;
    size_t offset = 0;

public:
    constexpr BinaryWriter(std::span<std::byte> buf) noexcept
    : buf(buf) {}

    template <class T>
    constexpr void write(T const &t) {
        if (offset + sizeof(T) > buf.size())
            throw std::out_of_range("BinaryWriter::write");
        auto tmp = reinterpret_cast<std::byte const *>(std::addressof(t));
        std::memcpy(buf.data() + offset, tmp, sizeof(T));
        offset += sizeof(T);
    }
};

struct BinaryExtensiveWriter {
    std::vector<std::byte> &buf;
    size_t offset = 0;

public:
    constexpr BinaryExtensiveWriter(std::vector<std::byte> &buf) noexcept
    : buf(buf) {}

    template <class T>
    constexpr void write(T const &t) {
        if (offset + sizeof(T) > buf.size())
            buf.resize(offset + sizeof(T)); /* amortized */
        auto tmp = reinterpret_cast<std::byte const *>(std::addressof(t));
        std::memcpy(buf.data() + offset, tmp, sizeof(T));
        offset += sizeof(T);
    }
};
