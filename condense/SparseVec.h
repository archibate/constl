#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

namespace condense {

template <class T, std::size_t N>
union UninitArray {
    T inner[N];

    UninitArray() {}

    [[nodiscard]] constexpr T &operator[](std::size_t i) noexcept {
        return inner[i];
    }

    [[nodiscard]] constexpr T const &operator[](std::size_t i) const noexcept {
        return inner[i];
    }

    template <class ...Ts>
    constexpr T *construct_at(std::size_t i, Ts &&...ts)
        noexcept(noexcept(std::construct_at(inner + i, std::forward<Ts>(ts)...)))
    {
        return std::construct_at(inner + i, std::forward<Ts>(ts)...);
    }

    constexpr void destroy_at(std::size_t i) noexcept {
        return std::destroy_at(inner + i);
    }
};

template <class T, class Id = std::size_t, class DenseId = std::size_t, class Alloc = std::allocator<T>>
struct SparseVec {
    struct SparseBlock {
        static const std::size_t N = 8;
        DenseId dids[1 << N];
        unsigned char bits[1 << (N - 3)];
        SparseBlock() noexcept : bits{} {}
    };

    struct DenseBlock {
        static const std::size_t N = 8;
        Id inds[1 << N];
        UninitArray<T, (1 << N)> vals;
        DenseBlock() noexcept {}
    };
    std::vector<SparseBlock, Alloc> m_sparse;
    std::vector<DenseBlock, Alloc> m_dense;
    DenseId m_densize;
    Id m_spsize;

    SparseVec() noexcept : m_densize(0), m_spsize(0) {}

    SparseVec(SparseVec &&that) noexcept
    : m_sparse(std::move(that.m_sparse))
    , m_dense(std::move(that.m_dense))
    , m_densize(std::exchange(that.m_densize, 0))
    , m_spsize(std::exchange(that.m_spsize, 0))
    {}

    SparseVec &operator=(SparseVec &&that) noexcept {
        if (this != &that) {
            m_sparse = std::move(that.m_sparse);
            m_dense = std::move(that.m_dense);
            m_densize = std::exchange(that.m_densize, 0);
            m_spsize = std::exchange(that.m_spsize, 0);
        }
        return *this;
    }

    SparseVec(SparseVec const &that) noexcept
    : m_sparse(that.m_sparse)
    , m_dense(that.m_dense)
    , m_densize(that.m_densize)
    , m_spsize(that.m_spsize)
    {}

    SparseVec &operator=(SparseVec const &that) noexcept {
        if (this != &that) {
            m_sparse = that.m_sparse;
            m_dense = that.m_dense;
            m_densize = that.m_densize;
            m_spsize = that.m_spsize;
        }
        return *this;
    }

    void swap(SparseVec &that) noexcept {
        if (this != &that) {
            std::swap(m_sparse, that.m_sparse);
            std::swap(m_dense, that.m_dense);
            std::swap(m_densize, that.m_densize);
            std::swap(m_spsize, that.m_spsize);
        }
    }

    void _sparse_emplace(Id id, DenseId did) noexcept {
        std::size_t blknr = id >> SparseBlock::N;
        std::size_t blkoff = id & ((1 << SparseBlock::N) - 1);
        if (m_sparse.size() <= blknr) {
            m_sparse.resize(blknr + 1);
        }
        SparseBlock &blk = m_sparse[blknr];
        blk.dids[blkoff] = did;
        blk.bits[blkoff >> 3] |= (1 << (blkoff & 7));
        ++m_spsize;
    }

    bool _sparse_erase(Id id, DenseId &did) noexcept {
        std::size_t blknr = (std::size_t)id >> SparseBlock::N;
        std::size_t blkoff = (std::size_t)id & ((1 << SparseBlock::N) - 1);
        if (m_sparse.size() <= blknr)
            return false;
        SparseBlock &blk = m_sparse[blknr];
        blk.dids[blkoff] = did;
        blk.bits[blkoff >> 3] &= ~(1 << (blkoff & 7));
        --m_spsize;
        return true;
    }

    bool _sparse_read(Id id, DenseId &did) noexcept {
        std::size_t blknr = (std::size_t)id >> SparseBlock::N;
        std::size_t blkoff = (std::size_t)id & ((1 << SparseBlock::N) - 1);
        if (m_sparse.size() <= blknr) {
            return false;
        }
        SparseBlock &blk = m_sparse[blknr];
        did = blk.dids[blkoff];
        return blk.bits[blkoff >> 3] & (1 << (blkoff & 7));
    }

    void _sparse_update(Id id, DenseId did) noexcept {
        std::size_t blknr = (std::size_t)id >> SparseBlock::N;
        std::size_t blkoff = (std::size_t)id & ((1 << SparseBlock::N) - 1);
        SparseBlock &blk = m_sparse[blknr];
        blk.dids[blkoff] = did;
    }

    void _dense_push_back(Id id, T val) noexcept {
        std::size_t blknr = (std::size_t)m_densize >> DenseBlock::N;
        std::size_t blkoff = (std::size_t)m_densize & ((1 << DenseBlock::N) - 1);
        if (m_dense.size() <= blknr)
            m_dense.resize(blknr + 1);
        DenseBlock &blk = m_dense[blknr];
        blk.inds[blkoff] = id;
        blk.vals.construct_at(blkoff, std::move(val));
        ++m_densize;
    }

    void _dense_pop_back() noexcept {
        std::size_t bblknr = (std::size_t)m_densize >> DenseBlock::N;
        std::size_t bblkoff = (std::size_t)m_densize & ((1 << DenseBlock::N) - 1);
        DenseBlock &bblk = m_dense[bblknr];
        bblk.vals.destroy_at(bblkoff);
        --m_densize;
    }

    Id _dense_swap_erase_back(Id id) noexcept {
        std::size_t blknr = (std::size_t)id >> DenseBlock::N;
        std::size_t blkoff = (std::size_t)id & ((1 << DenseBlock::N) - 1);
        DenseBlock &blk = m_dense[blknr];
        std::size_t bblknr = (std::size_t)m_densize >> DenseBlock::N;
        std::size_t bblkoff = (std::size_t)m_densize & ((1 << DenseBlock::N) - 1);
        DenseBlock &bblk = m_dense[bblknr];
        blk.vals.destroy_at(blkoff);
        blk.vals.construct_at(blkoff, std::move(bblk.vals[bblkoff]));
        bblk.vals.destroy_at(bblkoff);
        Id indb = bblk.inds[bblkoff];
        blk.inds[blkoff] = indb;
        --m_densize;
        return indb;
    }

    T &_dense_at(Id id) noexcept {
        std::size_t blknr = (std::size_t)id >> DenseBlock::N;
        std::size_t blkoff = (std::size_t)id & ((1 << DenseBlock::N) - 1);
        DenseBlock &blk = m_dense[blknr];
        return blk.vals[blkoff];
    }

    T const &_dense_at(Id id) const noexcept {
        std::size_t blknr = (std::size_t)id >> DenseBlock::N;
        std::size_t blkoff = (std::size_t)id & ((1 << DenseBlock::N) - 1);
        DenseBlock &blk = m_dense[blknr];
        return blk.vals[blkoff];
    }

    Id push_back(T val) noexcept {
        Id id = m_spsize;
        DenseId did = m_densize;
        _sparse_emplace(id, did);
        _dense_push_back(id, std::move(val));
        return id;
    }

    [[nodiscard]] T *at(Id id) noexcept {
        DenseId did;
        if (!_sparse_read(id, did)) {
            return nullptr;
        }
        return &_dense_at(did);
    }

    [[nodiscard]] T const *at(Id id) const noexcept {
        DenseId did;
        if (!_sparse_read(id, did)) {
            return nullptr;
        }
        return &_dense_at(did);
    }

    [[nodiscard]] bool contains(Id id) const noexcept {
        DenseId did;
        return _sparse_read(id, did);
    }

    void pop_back() noexcept {
        DenseId did;
        _sparse_erase(m_densize - 1, did);
        _dense_pop_back();
    }

    bool erase(Id id) noexcept {
        if (id + 1 == m_densize && m_densize != 0) {
            pop_back();
            return true;
        }
        DenseId did;
        if (!_sparse_erase(id, did)) {
            return false;
        }
        Id indb = _dense_swap_erase_back(id);
        _sparse_update(indb, did);
        return true;
    }

    template <class Fn>
    void foreach(Fn &&fn) noexcept {
        return _impl_foreach(*this, std::forward<Fn>(fn));
    }

    template <class Fn>
    void foreach(Fn &&fn) const noexcept {
        return _impl_foreach(*this, std::forward<Fn>(fn));
    }

    template <class Self, class Fn>
    static void _impl_foreach(Self &&self, Fn &&fn) noexcept {
        const std::size_t nblks = (std::size_t)self.m_densize >> DenseBlock::N;
        const std::size_t nrest = self.m_densize - (nblks << DenseBlock::N);
        if (nblks) {
            for (std::size_t blknr = 0; blknr != nblks; blknr++) {
                auto &blk = self.m_dense[blknr];
                for (std::size_t blkoff = 0; blkoff != (1 << DenseBlock::N); blkoff++) {
                    fn(blk.inds[blkoff], blk.vals[blkoff]);
                }
            }
        }
        if (nrest) {
            auto &bblk = self.m_dense[nblks];
            for (std::size_t bblkoff = 0; bblkoff != nrest; bblkoff++) {
                fn(bblk.inds[bblkoff], bblk.vals[bblkoff]);
            }
        }
    }

    template <class Fn>
    void foreach_block(Fn &&fn) noexcept {
        return _impl_foreach_block(*this, std::forward<Fn>(fn));
    }

    template <class Fn>
    void foreach_block(Fn &&fn) const noexcept {
        return _impl_foreach_block(*this, std::forward<Fn>(fn));
    }

    template <class Self, class Fn>
    static void _impl_foreach_block(Self &&self, Fn &&fn) noexcept {
        const std::size_t nblks = (std::size_t)self.m_densize >> DenseBlock::N;
        const std::size_t nrest = self.m_densize - (nblks << DenseBlock::N);
        if (nblks) {
            for (std::size_t blknr = 0; blknr != nblks; blknr++) {
                auto &blk = self.m_dense[blknr];
                fn(blk);
            }
        }
        if (nrest) {
            auto &bblk = self.m_dense[nblks];
            fn(bblk, nrest);
        }
    }

    DenseBlock &block(std::size_t blknr) noexcept {
        return m_dense[blknr];
    }

    DenseBlock const &block(std::size_t blknr) const noexcept {
        return m_dense[blknr];
    }

    static constexpr std::size_t block_size() noexcept {
        return DenseBlock::N;
    }

    std::size_t block_count() const noexcept {
        return (std::size_t)m_densize >> DenseBlock::N;
    }

    std::size_t last_block_size() const noexcept {
        return m_densize - (block_count() << DenseBlock::N);
    }

    [[nodiscard]] DenseId size() const noexcept {
        return m_densize;
    }

    [[nodiscard]] Id max_id() const noexcept {
        return m_spsize;
    }
};

}
