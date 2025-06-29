#pragma once

#include "reader.h"

namespace stream {
namespace detail {

template <typename Derived> struct IsDerivedFromReader {
  private:
    template <typename T> static std::true_type test(const Reader<T>*);

    static std::false_type test(...);

  public:
    static constexpr bool value =
        decltype(test(std::declval<Derived*>()))::value;
};

template <typename... ReaderTs>
struct AllDerivedFromReader
    : std::conjunction<std::integral_constant<
          bool, IsDerivedFromReader<ReaderTs>::value>...> {};

template <typename T> struct ReaderValue {
    using type = void;
};

template <typename T> struct ReaderValue<Reader<T>> {
    using type = T;
};

template <typename Derived> struct ExtractReaderType {
  private:
    template <typename T_> static Reader<T_>* test(Reader<T_>*);

    static void* test(...);

    using base = decltype(test(std::declval<Derived*>()));

  public:
    using type = typename ReaderValue<std::remove_pointer_t<base>>::type;
};

template <typename ReaderT>
using ReaderDataT = typename ExtractReaderType<ReaderT>::type;

} // namespace detail
} // namespace stream