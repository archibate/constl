#include <utility>
#include <memory>

template <class T>
struct deepcopy_ptr : public std::unique_ptr<T> {
    using std::unique_ptr<T>::unique_ptr;

    deepcopy_ptr(std::unique_ptr<T> up)
    : std::unique_ptr<T>(std::move(up))
    {}

    deepcopy_ptr(deepcopy_ptr const &that)
    : std::unique_ptr<T>(that ? std::make_unique<T>(*that) : nullptr)
    {}
};
