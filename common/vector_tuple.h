#pragma once

#include <vector>

namespace conlib {

template <class... Ts> struct VectorTuple {
  explicit VectorTuple(size_t cap = 0) {}

  VectorTuple() = default;
};

template <class T, class... Ts> struct VectorTuple<T, Ts...> : VectorTuple<Ts...> {
  explicit VectorTuple(size_t cap) : VectorTuple<Ts...>(cap), tail() { tail.reserve(cap); }

  VectorTuple() : VectorTuple<Ts...>(), tail() {}

  std::vector<T> tail;
};

template <size_t, class> struct ElemTypeHolder;

template <class T, class... Ts> struct ElemTypeHolder<0, VectorTuple<T, Ts...>> {
  typedef std::vector<T> type;
};

template <size_t k, class T, class... Ts> struct ElemTypeHolder<k, VectorTuple<T, Ts...>> {
  typedef typename ElemTypeHolder<k - 1, VectorTuple<Ts...>>::type type;
};

template <class T> void emplace(VectorTuple<T> &v, T &&t) {
  v.tail.emplace_back(std::forward<T>(t));
}

template <class T, class... Ts> void emplace(VectorTuple<T, Ts...> &v, T &&t, Ts &&... ts) {
  v.tail.emplace_back(std::forward<T>(t));
  VectorTuple<Ts...> &base = v;
  emplace(base, std::forward<Ts>(ts)...);
}

template <size_t k, class... Ts>
typename std::enable_if<k == 0, typename ElemTypeHolder<0, VectorTuple<Ts...>>::type &>::type
visit(VectorTuple<Ts...> &t) {
  return t.tail;
}

template <size_t k, class T, class... Ts>
typename std::enable_if<k != 0, typename ElemTypeHolder<k, VectorTuple<T, Ts...>>::type &>::type
visit(VectorTuple<T, Ts...> &t) {
  VectorTuple<Ts...> &base = t;
  return visit<k - 1>(base);
}

} // namespace conlib