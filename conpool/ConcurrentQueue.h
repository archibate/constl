#pragma once

#include <type_traits>
#include <atomic>
#include <optional>
#include <memory>

namespace conpool {

template <class T>
struct ConcurrentQueue {
    struct Node {
        union {
            T value;
        };
        std::atomic<Node *> next{reinterpret_cast<Node *>(-1)};
        Node() {}
        ~Node() {}
    };
    std::atomic<Node *> head{nullptr};

    void push(auto &&...args) requires (std::constructible_from<T, decltype(args)...>) {
        Node *hptr, *node;
        node = new Node;
        struct exception_guard {
            Node *ptr;
            ~exception_guard() noexcept {
                delete ptr;
            }
        } exguard{node};
        std::construct_at(std::addressof(node->value), std::forward<decltype(args)>(args)...);
        exguard.ptr = nullptr;
        hptr = head.load(std::memory_order_acquire);
        while (!head.compare_exchange_weak(hptr, node, std::memory_order_release, std::memory_order_acquire));
        node->next.store(hptr, std::memory_order_release);
    }

    std::optional<T> try_pop() noexcept {
        Node *hptr, *next;
        hptr = head.load(std::memory_order_acquire);
        do {
            if (!hptr) return std::nullopt;
            do {
                next = hptr->next.load(std::memory_order_acquire);
            } while (next == reinterpret_cast<Node *>(-1));
        } while (!head.compare_exchange_weak(hptr, next, std::memory_order_release, std::memory_order_acquire));
        auto result = std::optional<T>(std::move(hptr->value));
        std::destroy_at(std::addressof(hptr->value));
        delete hptr;
        return result;
    }
};

}
