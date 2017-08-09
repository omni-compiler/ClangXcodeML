#ifndef UTIL_H
#define UTIL_H

template<typename K, typename V>
llvm::Optional<V> getOrNull(const std::map<K,V>& m, const K& key) {
  using MaybeV = llvm::Optional<V>;
  const auto iter = m.find(key);
  if (iter == m.end()) {
    return MaybeV();
  }
  return static_cast<MaybeV>(iter->second);
}

#endif /* !UTIL_H */
