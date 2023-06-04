#include <array>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <tuple>

namespace morton3d {

constexpr static uint64_t encode1(uint64_t x) {
    x = (x | (x << 32)) & 0x7fff00000000ffff; // 0b0111111111111111000000000000000000000000000000001111111111111111
    x = (x | (x << 16)) & 0x00ff0000ff0000ff; // 0b0000000011111111000000000000000011111111000000000000000011111111
    x = (x | (x <<  8)) & 0x700f00f00f00f00f; // 0b0111000000001111000000001111000000001111000000001111000000001111
    x = (x | (x <<  4)) & 0x30c30c30c30c30c3; // 0b0011000011000011000011000011000011000011000011000011000011000011
    x = (x | (x <<  2)) & 0x1249249249249249; // 0b0001001001001001001001001001001001001001001001001001001001001001
    return x;
}

constexpr static uint64_t encode(uint64_t x, uint64_t y, uint64_t z) {
    return encode1(x) | encode1(y << 1) | encode1(z << 2);
}

constexpr static uint64_t decode1(uint64_t x) {
    x = x & UINT64_C(0x9249249249249249);
    x = (x | (x >>  2)) & UINT64_C(0x30c30c30c30c30c3);
    x = (x | (x >>  4)) & UINT64_C(0xf00f00f00f00f00f);
    x = (x | (x >>  8)) & UINT64_C(0x00ff0000ff0000ff);
    x = (x | (x >> 16)) & UINT64_C(0xffff00000000ffff);
    x = (x | (x >> 32)) & UINT64_C(0xffffffffffffffff);
    return x;
}

constexpr static std::tuple<uint64_t, uint64_t, uint64_t> decode(uint64_t d) {
    return {decode1(d), decode1(d >> 1), decode1(d >> 2)};
}

}

struct Block {
    int data[258];
};

struct vec3 {
    int x, y, z;
};

int morton(vec3 const &v) {
    return morton3d::encode(v.x, v.y, v.z);
}

int morton_xp(int m) {
    return m + 1;
}

int morton_xm(int m) {
    return m - 1;
}

int morton_yp(int m) {
    return m + (1 << 10);
}

int morton_ym(int m) {
    return m - (1 << 10);
}

int morton_zp(int m) {
    return m + (1 << 20);
}

int morton_zm(int m) {
    return m - (1 << 20);
}

int main() {
    std::vector<vec3> pars;
    std::unordered_map<int, Block> lut;
    for (int i = 0; i < pars.size(); i++) {
        int m = morton(pars[i]);
        lut[m >> 8].data[1 + (m & 255)] = i;
    }
    for (auto &[him, blk]: lut) {
        for (int lom = 0; lom < 256; lom++) {
            int m = him << 8 | lom;
            int i = blk.data[1 + lom];
            int loxp = morton_xp(lom);
            int ixp = blk.data[1 + morton_xp(lom)];
            int ixm = blk.data[1 + morton_xm(lom)];
            int iyp = blk.data[1 + morton_yp(lom)];
            int iym = blk.data[1 + morton_ym(lom)];
            int izp = blk.data[1 + morton_zp(lom)];
            int izm = blk.data[1 + morton_zm(lom)];
        }
    }
}
