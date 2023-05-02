#pragma once

#include <optional>

namespace constl {

template <class M>
constexpr typename M::mapped_type map_get
( M const &m
, typename M::key_type const &key
, typename M::mapped_type const &defl
) {
  typename M::const_iterator it = m.find(key);
  if (it != m.end()) {
    return it->second;
  } else {
    return defl;
  }
}

template <class M>
constexpr std::optional<typename M::mapped_type> map_get
( M const &m
, typename M::key_type const &key
) {
  typename M::const_iterator it = m.find(key);
  if (it != m.end()) {
    return it->second;
  } else {
    return std::nullopt;
  }
}

// Usage:
//
// map<string, string> m;
//
// string val = map_get(m, "key").value_or("default");
//
// print(val);  // "default"

}
