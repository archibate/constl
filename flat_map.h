#pragma once

#include <memory>
#include <utility>
#include <concepts>
#include "hash.h"

namespace constl {

namespace _flat_map_details {

template
< class K
, class V
, class Hash = generic_hash<K>
, class KeyEq = std::equal_to<K>
, class Alloc = std::allocator<std::pair<const K, V>>
>
class flat_map {
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using hasher = Hash;
    using key_equal = KeyEq;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using allocator_type = Alloc;

private:
    using AllocTrait = std::allocator_traits<allocator_type>;
    using AllocU8Type = typename AllocTrait::template rebind_alloc<uint8_t>;
    using AllocU8Trait = std::allocator_traits<AllocU8Type>;
    using AllocKeyType = typename AllocTrait::template rebind_alloc<key_type>;
    using AllocKeyTrait = std::allocator_traits<AllocKeyType>;
    using AllocMappedType = typename AllocTrait::template rebind_alloc<key_type>;
    using AllocMappedTrait = std::allocator_traits<AllocMappedType>;

    template <class T>
    class IteratorBase {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::pair<const K, V>;
        using reference = std::pair<const K &, V &>;
    
    private:
        constexpr explicit IteratorBase
        ( size_t index
        , flat_map const &parent
        ) noexcept
        : m_keys(parent.m_keys + index)
        , m_vals(parent.m_vals + index)
        {}

    public:
        constexpr bool operator!=(IteratorBase const &that) const noexcept {
            return m_keys != that.m_keys;
        }

        constexpr bool operator==(IteratorBase const &that) const noexcept {
            return !(*this != that);
        }
    
        constexpr IteratorBase &operator--() noexcept {
            go_back();
            return *this;
        }
    
        constexpr IteratorBase operator--(int) noexcept {
            IteratorBase old = *this;
            --*this;
            return old;
        }
    
        constexpr IteratorBase &operator++() noexcept {
            go_forward();
            return *this;
        }
    
        constexpr IteratorBase operator++(int) noexcept {
            IteratorBase old = *this;
            ++*this;
            return old;
        }

        constexpr reference operator*() const noexcept {
            return {*m_keys, *m_vals};
        }

    private:
        constexpr void go_back() noexcept {
            --m_keys;
            --m_vals;
        }

        constexpr void go_forward() noexcept {
            ++m_keys;
            ++m_vals;
        }

        key_type *m_keys;
        mapped_type *m_vals;

        friend flat_map;
    };

    template <class K2 = key_type>
    constexpr std::pair<size_t, bool> bucket_index_on
    ( value_type *keys
    , value_type *vals
    , uint8_t *bitmaps
    , size_t bucket_count
    , K2 const &k
    ) const noexcept {
        if (!bucket_count) {
            return {(size_t)-1, false};
        }
        size_t h = m_hash(k) % bucket_count;
        while (bitmaps[h >> 3] & (1 << (h & 7))) {
            if (m_key_eq(k, keys[h]))
                return {h, true};
            h = (h + 1) % bucket_count;
        }
        return {h, false};
    }

public:
    using iterator = IteratorBase<value_type>;
    using const_iterator = IteratorBase<const value_type>;

    constexpr allocator_type get_allocator() const noexcept {
        return allocator_type();
    }

    constexpr key_equal key_eq() const noexcept {
        return m_key_eq;
    }

    constexpr hasher hash_function() const noexcept {
        return m_hash;
    }

    constexpr iterator begin() noexcept {
        return iterator(*this, 0);
    }

    constexpr iterator end() noexcept {
        return iterator(*this, m_bucket_count);
    }

    constexpr const_iterator begin() const noexcept {
        return const_iterator(*this, 0);
    }

    constexpr const_iterator end() const noexcept {
        return const_iterator(*this, m_bucket_count);
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    constexpr const_iterator cend() const noexcept {
        return end();
    }

    template <class K2 = key_type>
    constexpr std::pair<size_t, bool> bucket_index(K2 const &k) const noexcept {
        return bucket_index_on(m_keys, m_vals, m_bitmaps, m_bucket_count, k);
    }

    constexpr std::pair<iterator, bool> insert(value_type &&kv) {
        reserve(m_size + 1);
        std::pair<size_t, bool> bi = bucket_index(kv.first);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            m_bitmaps[h >> 3] |= 1 << (h & 7);
            std::construct_at(m_keys + h, std::move(kv.first));
            std::construct_at(m_vals + h, std::move(kv.second));
            ++m_size;
        }
        return {iterator(*this, h), !found};
    }

    constexpr std::pair<iterator, bool> insert(value_type const &kv) {
        reserve(m_size + 1);
        std::pair<size_t, bool> bi = bucket_index(kv.first);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            m_bitmaps[h >> 3] |= 1 << (h & 7);
            std::construct_at(m_keys + h, std::move(kv.first));
            std::construct_at(m_vals + h, std::move(kv.second));
            ++m_size;
        }
        return {iterator(*this, h), !found};
    }

    template <class K2 = key_type>
    constexpr bool erase(K2 const &k) noexcept {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        if (found) {
            m_bitmaps[h >> 3] &= ~(1 << (h & 7));
            std::destroy_at(m_keys + h);
            std::destroy_at(m_vals + h);
            --m_size;
            return true;
        }
        return false;
    }

    constexpr iterator erase(iterator pos) noexcept {
        size_t h = pos.bucket_index();
        m_bitmaps[h >> 3] &= ~(1 << (h & 7));
        std::destroy_at(m_keys + h);
        std::destroy_at(m_vals + h);
        --m_size;
        return pos;
    }

    template <class K2 = key_type>
    constexpr bool contains(K2 const &k) const noexcept {
        return bucket_index(k).second;
    }

    template <class K2 = key_type>
    constexpr iterator find(K2 const &k) noexcept {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            return end();
        }
        return iterator(*this, h);
    }

    template <class K2 = key_type>
    constexpr const_iterator find(K2 const &k) const noexcept {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            return end();
        }
        return const_iterator(*this, h);
    }

    template <class K2 = key_type>
    constexpr mapped_type &at(K2 const &k) {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        [[unlikely]] if (!found) {
            throw std::out_of_range("flat_map::at");
        }
        return m_vals[h];
    }

    template <class K2 = key_type>
    constexpr mapped_type const &at(K2 const &k) const {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        [[unlikely]] if (!found) {
            throw std::out_of_range("flat_map::at");
        }
        return m_vals[h];
    }

    constexpr mapped_type &operator[](key_type const &k) {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            if (m_size + 1 > capacity()) {
                reserve(m_size + 1);
                bi = bucket_index(k);
                h = bi.first;
                found = bi.second;
            }
            m_bitmaps[h >> 3] |= 1 << (h & 7);
            std::construct_at(m_keys + h, k);
            std::construct_at(m_vals + h);
            ++m_size;
        }
        return m_keys[h];
    }

    template <class V2 = mapped_type>
    constexpr std::pair<iterator, bool> insert_or_assign(key_type const &k, V2 &&v) {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            if (m_size + 1 > capacity()) {
                reserve(m_size + 1);
                bi = bucket_index(k);
                h = bi.first;
                found = bi.second;
            }
            m_bitmaps[h >> 3] |= 1 << (h & 7);
            std::construct_at(m_keys + h, k);
            std::construct_at(m_vals + h, std::forward<V2>(v));
            ++m_size;
        } else {
            m_vals[h] = std::forward<V2>(v);
        }
        return {iterator(*this, h), !found};
    }

    template <class V2 = mapped_type>
    constexpr std::pair<iterator, bool> insert_or_assign(key_type &&k, V2 &&v) {
        std::pair<size_t, bool> bi = bucket_index(k);
        size_t h = bi.first;
        bool found = bi.second;
        if (!found) {
            if (m_size + 1 > capacity()) {
                reserve(m_size + 1);
                bi = bucket_index(k);
                h = bi.first;
                found = bi.second;
            }
            m_bitmaps[h >> 3] |= 1 << (h & 7);
            std::construct_at(m_keys + h, k);
            std::construct_at(m_vals + h, std::forward<V2>(v));
            ++m_size;
        } else {
            m_vals[h] = std::forward<V2>(v);
        }
        return {iterator(*this, h), !found};
    }

    constexpr void reserve(size_t n) {
        if (n >= capacity()) rehash(std::max(n * 2, m_bucket_count * 2));
    }

    constexpr void shrink_to_fit() {
        if (m_bucket_count > m_size * 2) rehash(0);
    }

    constexpr void rehash(size_t n) {
        n = std::max(n, m_size * 2);
        if (n) {
            key_type *keys = AllocKeyTrait::allocate(m_alloc_k, n);
            mapped_type *vals = AllocMappedTrait::allocate(m_alloc_v, n);
            uint8_t *bitmaps = AllocU8Trait::allocate(m_alloc_u8, (n + 7) / 8);
            for (size_t i = 0; i < (n + 7) / 8; i++) bitmaps[i] = 0;
            if (m_bucket_count) {
                for (size_t h = 0; h < m_bucket_count; h++) {
                    if (!(m_bitmaps[h >> 3] & (1 << (h & 7)))) continue;
                    size_t h2 = bucket_index_on(keys, vals, bitmaps, n, m_keys[h]).first;
                    std::construct_at(keys + h2, std::move(keys[h]));
                    std::construct_at(vals + h2, std::move(vals[h]));
                    std::destroy_at(m_keys + h);
                    bitmaps[h2 >> 3] |= 1 << (h2 & 7);
                }
                AllocKeyTrait::deallocate(m_alloc_k, m_keys, m_bucket_count);
                AllocMappedTrait::deallocate(m_alloc_v, m_vals, m_bucket_count);
                AllocU8Trait::deallocate(m_alloc_u8, m_bitmaps, (m_bucket_count + 7) / 8);
            }
            m_bucket_count = n;
            m_keys = keys;
            m_vals = vals;
            m_bitmaps = bitmaps;
        }
    }

    constexpr size_t size() const noexcept {
        return m_size;
    }

    constexpr size_t capacity() const noexcept {
        return (m_bucket_count + 1) / 2;
    }

    constexpr float load_factor() const noexcept {
        return (float)m_size / (float)std::max(m_bucket_count, (size_t)1);
    }

    constexpr float max_load_factor() const noexcept {
        return 0.5f;
    }

    constexpr size_t bucket_count() const noexcept {
        return m_bucket_count;
    }

    constexpr flat_map()
        : m_keys(nullptr)
        , m_vals(nullptr)
        , m_bitmaps(nullptr)
        , m_bucket_count(0)
        , m_size(0)
    {}

    template <std::input_iterator InputIt, std::sentinel_for<InputIt> InputSen>
    constexpr flat_map(InputIt first, InputSen last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    template <std::input_iterator InputIt, std::sentinel_for<InputIt> InputSen>
    constexpr void insert(InputIt first, InputSen last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    constexpr void clear() noexcept {
        m_size = 0;
        for (size_t h = 0; h < m_bucket_count; h++) {
            if (m_bitmaps[h >> 3] & (1 << (h & 7))) {
                m_bitmaps[h >> 3] &= ~(1 << (h & 7));
                std::destroy_at(m_keys + h);
                std::destroy_at(m_vals + h);
            }
        }
    }

    constexpr ~flat_map() noexcept {
        if (m_bucket_count) {
            for (size_t h = 0; h < m_bucket_count; h++) {
                if (m_bitmaps[h >> 3] & (1 << (h & 7))) {
                    std::destroy_at(m_keys + h);
                    std::destroy_at(m_vals + h);
                }
            }
            AllocKeyTrait::deallocate(m_alloc_k, m_keys, m_bucket_count);
            AllocMappedTrait::deallocate(m_alloc_v, m_vals, m_bucket_count);
            AllocU8Trait::deallocate(m_alloc_u8, m_bitmaps, (m_bucket_count + 7) / 8);
            m_keys = nullptr;
            m_vals = nullptr;
            m_bitmaps = nullptr;
            m_bucket_count = 0;
        }
    }

    constexpr flat_map(flat_map &&that) noexcept {
        m_keys = that.m_keys;
        that.m_keys = nullptr;
        m_vals = that.m_vals;
        that.m_vals = nullptr;
        m_bitmaps = that.m_bitmaps;
        that.m_bitmaps = nullptr;
        m_bucket_count = that.m_bucket_count;
        that.m_bucket_count = 0;
        m_size = that.m_size;
        that.m_size = 0;
    }

    constexpr flat_map &operator=(flat_map &&that) noexcept {
        for (size_t h = 0; h < m_bucket_count; h++) {
            if (m_bitmaps[h >> 3] & (1 << (h & 7))) {
                std::destroy_at(m_keys + h);
                std::destroy_at(m_vals + h);
            }
        }
        AllocKeyTrait::deallocate(m_alloc_k, m_keys, m_bucket_count);
        AllocMappedTrait::deallocate(m_alloc_v, m_vals, m_bucket_count);
        AllocU8Trait::deallocate(m_alloc_u8, m_bitmaps, (m_bucket_count + 7) / 8);

        m_keys = that.m_keys;
        that.m_keys = nullptr;
        m_vals = that.m_vals;
        that.m_vals = nullptr;
        m_bitmaps = that.m_bitmaps;
        that.m_bitmaps = nullptr;
        m_bucket_count = that.m_bucket_count;
        that.m_bucket_count = 0;
        m_size = that.m_size;
        that.m_size = 0;
        return *this;
    }

    constexpr key_type *key_data() {
        return m_keys;
    }

    constexpr key_type const *key_data() const {
        return m_keys;
    }

    constexpr mapped_type *mapped_data() {
        return m_vals;
    }

    constexpr mapped_type const *mapped_data() const {
        return m_vals;
    }

private:
    key_type *m_keys;
    mapped_type *m_vals;
    uint8_t *m_bitmaps;
    size_t m_bucket_count;
    size_t m_size;
    [[no_unique_address]] hasher m_hash;
    [[no_unique_address]] key_equal m_key_eq;
    [[no_unique_address]] AllocKeyType m_alloc_k;
    [[no_unique_address]] AllocMappedType m_alloc_v;
    [[no_unique_address]] AllocU8Type m_alloc_u8;
};

}

using _flat_map_details::flat_map;

}
