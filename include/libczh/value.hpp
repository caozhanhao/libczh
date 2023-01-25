//   Copyright 2021-2023 libczh - caozhanhao
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
#ifndef LIBCZH_VALUE_HPP
#define LIBCZH_VALUE_HPP
#pragma once

#include "error.hpp"
#include <variant>
#include <string>
#include <vector>
#include <typeinfo>
#include <type_traits>
#include <ostream>
#include <deque>
#include <set>
#include <queue>
#include <array>
#include <list>

namespace czh
{
  namespace node
  {
    class Node;
  }
  namespace value
  {
    class Null
    {
    public:
      bool operator==(const Null &) const { return true; }
    };
    
    class Reference
    {
    public:
      std::vector<std::string> path;
      
      Reference() = delete;
      
      Reference(std::vector<std::string> v) : path(std::move(v)) {}
      
      bool operator==(const Reference &ref) const
      {
        // Sometimes path doesn't matter.
        // Not use. (See as Node::operator==)
        return path == ref.path;
      }
    };
    
    namespace details
    {
      template<typename... List>
      struct TypeList {};
      
      struct TypeListError {};
      
      template<typename... List>
      std::variant<List...> as_variant(TypeList<List...>);
      
      template<typename List1, typename List2>
      struct link;
      template<typename ... Args1, typename ... Args2>
      struct link<TypeList<Args1...>, TypeList<Args2...>>
      {
        using type = TypeList<Args1..., Args2...>;
      };
      
      template<typename L1, typename L2>
      using link_t = typename link<L1, L2>::type;
      
      template<typename T, typename List>
      struct contains : std::true_type {};
      template<typename T, typename First, typename... Rest>
      struct contains<T, TypeList<First, Rest...>>
          : std::conditional<std::is_same_v<T, First>, std::true_type,
              contains<T, TypeList<Rest...>>>::type
      {
      };
      template<typename T>
      struct contains<T, TypeList<>> : std::false_type {};
      
      template<typename T, typename List>
      constexpr bool contains_v = contains<T, List>::value;
      
      // adapted from: https://zh.cppreference.com/w/cpp/types/is_convertible
      template<typename From, typename To>
      auto is_czh_convertible_helper(int)
      -> decltype(void(std::declval<void (&)(To)>()(std::declval<From>())), std::true_type{});
      
      template<typename, typename>
      auto is_czh_convertible_helper(...) -> std::false_type;
      
      template<typename From, typename To, typename U = void>
      struct is_czh_convertible : public std::false_type {};
      template<typename From, typename To>
      requires (decltype(is_czh_convertible_helper<From, To>(0))::value
                && decltype(is_czh_convertible_helper<To, From>(0))::value)
      struct is_czh_convertible<From, To> : public std::true_type {};
      template<typename From, typename To>
      constexpr bool is_czh_convertible_v = is_czh_convertible<From, To>::value;
      
      template<typename T, typename List>
      struct conv_contains : std::true_type {};
      template<typename T, typename First, typename... Rest>
      struct conv_contains<T, TypeList<First, Rest...>>
          : std::conditional<std::is_same_v<T, First> || is_czh_convertible_v<T, First>, std::true_type,
              conv_contains<T, TypeList<Rest...>>>::type
      {
      };
      template<typename T>
      struct conv_contains<T, TypeList<>> : std::false_type {};
      
      template<typename T, typename List>
      constexpr bool conv_contains_v = conv_contains<T, List>::value;
      
      template<typename T, typename List>
      struct index_of;
      template<typename First, typename ... Rest>
      struct index_of<First, TypeList<First, Rest...>>
      {
        static constexpr int value = 0;
      };
      template<typename T>
      struct index_of<T, TypeList<>>
      {
        static constexpr int value = -1;
      };
      template<typename T, typename First, typename ...Rest>
      struct index_of<T, TypeList<First, Rest...>>
      {
        static constexpr int temp = index_of<T, TypeList<Rest...>>::value;
        static constexpr int value = temp == -1 ? -1 : temp + 1;
      };
      
      template<typename T, typename List>
      constexpr int index_of_v = index_of<T, List>::value;
      
      template<int index, typename List>
      struct index_at;
      template<int index>
      struct index_at<index, TypeList<>>
      {
        using type = TypeListError;
      };
      template<typename First, typename ... Rest>
      struct index_at<0, TypeList<First, Rest...>>
      {
        using type = First;
      };
      template<int index, typename First, typename ... Rest>
      struct index_at<index, TypeList<First, Rest...>>
      {
        using type = typename index_at<index - 1, TypeList<Rest...>>::type;
      };
      
      template<int index, typename List>
      using index_at_t = typename index_at<index, List>::type;
      
      template<typename List, size_t sz = 0>
      struct size_of;
      template<size_t sz>
      struct size_of<TypeList<>, sz>
      {
        static constexpr size_t value = sz;
      };
      template<typename First, typename ...Rest, size_t sz>
      struct size_of<TypeList<First, Rest...>, sz>
      {
        static constexpr size_t value = size_of<TypeList<Rest...>>::value + 1;
      };
      
      template<typename List>
      constexpr size_t size_of_v = size_of<List>::value;
  
  
      template<typename T>
      struct info_of_array;
      template<typename T, std::size_t sz>
      struct info_of_array<T[sz]>
      {
        static const std::size_t value = sz;
        using type = T;
      };
      template<typename T>
      constexpr size_t size_of_array_v = info_of_array<T>::value;
  
      template<typename T>
      using type_of_array_t = typename info_of_array<T>::type;
  
      using BasicVTList = TypeList<Null, int, long long, double, bool, std::string>;
      using BasicVT = decltype(as_variant(BasicVTList{}));
      // In some older compilers, const char* will implicitly convert to bool when
      // constructing a std::variant<std::string, bool>.
      // IOW, BasicVT{"1"} stored a bool previously.
      // Now this clearly unwanted conversion is fixed:
      // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0608r3.html#Problems-to-solve
  
      constexpr size_t basic_vtlist_size = size_of_v<BasicVTList>;
  
      using Array = std::vector<BasicVT>;//insert() begin() end()
      
      using HighVTList = TypeList<Reference, Array>;
      using VTList = link_t<BasicVTList, HighVTList>;
      using VT = decltype(as_variant(VTList{}));
  
      template<typename T>
      consteval std::string_view nameof()
      {
        std::string_view str = std::experimental::source_location::current().function_name();
        auto b = str.find_first_of('<');
        auto e = str.find_last_of('>');
        return str.substr(b + 1, e - b - 1);
      }
      
      std::string get_typename(size_t sz)
      {
        static std::vector<std::string>
            names{"Null", "int", "long long", "double", "bool", "std::string", "czh::value::Reference",
                  "czh::value::Array"};
        return names[sz];
      }
      
      template<typename T, typename U = void>
      struct is_czh_type : public std::false_type {};
      template<typename T>
      struct is_czh_type<T, std::enable_if_t<conv_contains_v<std::decay_t<T>, VTList>>>
          : public std::true_type
      {
      };
      template<typename T>
      constexpr bool is_czh_type_v = is_czh_type<T>::value;
      
      template<typename T, typename U = void>
      struct is_czh_basic_type : public std::false_type {};
      template<typename T>
      struct is_czh_basic_type<T, std::enable_if_t<conv_contains_v<std::decay_t<T>, BasicVTList>>>
          : public std::true_type
      {
      };
      template<typename T>
      constexpr bool is_czh_basic_type_v = is_czh_type<T>::value;
      
      // adapted from: https://www.zhihu.com/question/357829786
      template<class BeginIt, class EndIt>
      concept CzhItRange =
      requires(BeginIt begin_it, EndIt end_it)
      {
        { ++begin_it };
        { *begin_it };
        requires !std::is_void_v<decltype(*begin_it)>;
        requires is_czh_basic_type_v<decltype(*begin_it)>;
        { begin_it != end_it };
      };
      template<class T>
      concept CzhContainer = (!std::is_same_v<std::decay_t<T>, std::string>)
                             && (
                                 (std::is_array_v<T> && is_czh_basic_type_v<std::remove_pointer_t<T>>)
                                 ||
                                 requires(T value)
                                 {
                                   { value.begin() };
                                   { value.end() };
                                   requires CzhItRange<decltype(value.begin()), decltype(value.end())>;
                                 }
                                 ||
                                 requires(T value)
                                 {
                                   { std::begin(value) };
                                   { std::end(value) };
                                   requires CzhItRange<decltype(std::begin(value)), decltype(std::end(value))>;
                                 }
                             );
      
      
      template<typename T, typename U = void>
      struct is_czh_container : public std::false_type {};
      template<typename T> requires CzhContainer<T>
      struct is_czh_container<T> : public std::true_type {};
      
      template<typename T>
      constexpr bool is_czh_container_v = is_czh_container<T>::value;
      
      struct ValueTag {};
      struct NormalArrayTag {};
      struct CppArrayTag {};
      struct AnyArrayTag {};
      template<typename T>
      struct TagDispatch
      {
        using tag = std::conditional_t<
            std::is_same_v<std::decay_t<T>, Array> || is_czh_convertible_v<std::decay_t<T>, Array>, AnyArrayTag,
            std::conditional_t<is_czh_container_v<T>,
                std::conditional_t<std::is_array_v<T>, CppArrayTag, NormalArrayTag>, ValueTag>>;
      };
    }
    using details::Array;
  
    template<typename T>
    concept CzhValueType = details::is_czh_type_v<T>;
    template<typename T>
    concept CzhGetType = details::is_czh_type_v<T> ||
                         (details::is_czh_container_v<T> && !std::is_array_v<std::decay_t<T>>);
  
    class Value;
  
    template<typename T>
    concept CzhAssignType = (details::is_czh_type_v<T> || details::is_czh_container_v<T>)
                            && (!std::is_base_of_v<Value, std::decay_t<T>>)
                            // avoid conflicts with other assignment operator
                            && (!std::is_same_v<char *, std::remove_cvref_t<std::decay_t<T>>>);
    // char* should be converted to std::string.
  
    class Value
    {
    private:
      details::VT value;
    public:
      template<CzhAssignType T>
      explicit Value(T &&data)
      {
        *this = std::forward<T>(data);
      }
    
      explicit Value(const char *c) : value(std::string(c)) {}
    
      Value(Value &&) = default;
    
      explicit Value(const Value &) = default;
    
      Value &operator=(Value &&v) = default;
    
      explicit Value() : value(Null()) {}
    
      bool operator==(const Value &v) const
      {
        return value == v.value;
      }
    
      template<CzhGetType T>
      [[nodiscard]]T get(const std::experimental::source_location &l =
      std::experimental::source_location::current()) const
      {
        return internal_get<T>(typename details::TagDispatch<T>::tag{}, l);
      }
    
      template<CzhAssignType T>
      Value &operator=(T &&v)
      {
        internal_assign<T>(std::forward<T>(v), typename details::TagDispatch<T>::tag{});
        return *this;
      }
  
      Value &operator=(const char *v)
      {
        value = std::string(v);
        return *this;
      }
    
      template<CzhValueType T>
      [[nodiscard]]bool is() const
      {
        return value.index() == details::index_of_v<T, details::VTList>;
      }
    
      template<CzhGetType T>
      [[nodiscard]]bool can_get() const
      {
        return internal_can_get<T>(typename details::TagDispatch<T>::tag{});
      }
    
      [[nodiscard]] auto get_variant() const
      {
        return value;
      }
    
      [[nodiscard]] std::string get_typename() const { return details::get_typename(value.index()); }
  
    private:
      template<CzhGetType T>
      [[nodiscard]] bool internal_can_get(details::ValueTag) const { return is<T>(); }
    
      template<CzhGetType T>
      [[nodiscard]] bool internal_can_get(details::AnyArrayTag) const { return is<Array>(); }
    
      template<CzhGetType T>
      [[nodiscard]] bool internal_can_get(details::NormalArrayTag) const { return is<Array>(); }
    
      template<CzhGetType T>
      [[nodiscard]]T internal_get(details::ValueTag, const std::experimental::source_location &l) const
      {
        if (!is<T>()) get_error_index<T>(l);
        return std::get<T>(value);
      }
    
      template<CzhGetType T>
      void narrow_transfrom_to_container(const Array &from, T &to) const
      {
        for (auto &r: from)
        {
          auto pval = std::get_if<typename T::value_type>(&r);
          error::czh_assert(pval, "This array contains different types.");
          to.insert(std::end(to), *pval);
        }
      }
    
      template<CzhGetType T>
      [[nodiscard]]T internal_get(details::NormalArrayTag, const std::experimental::source_location &l) const
      {
        if (!is<Array>()) get_error_index<T>(l);
        auto &varr = std::get<Array>(value);
        T ret;
        narrow_transfrom_to_container(varr, ret);
        return std::move(ret);
      }
    
      template<CzhGetType T>
      [[nodiscard]]Array internal_get(details::AnyArrayTag, const std::experimental::source_location &l) const
      {
        if (!is<Array>()) get_error_index<T>(l);
        return std::get<Array>(value);
      }
    
      template<CzhAssignType T>
      void internal_assign(T &&v, details::ValueTag) { value = std::forward<T>(v); }
    
      template<CzhAssignType T>
      void internal_assign(T &&v, details::AnyArrayTag) { value = std::forward<T>(v); }
    
      template<CzhAssignType T>
      void internal_assign(T &&v, details::NormalArrayTag)
      {
        Array tmp;
        for (auto r: v)
        {
          tmp.emplace_back(std::move(r));
        }
        value = std::move(tmp);
      }
    
      template<CzhAssignType T>
      void internal_assign(T &&v, details::CppArrayTag)
      {
        Array tmp;
        for (size_t i = 0; i < details::size_of_array_v<T>; ++i)
        {
          tmp.emplace_back(v[i]);
        }
        value = std::move(tmp);
      }
    
      template<CzhGetType T>
      void get_error_index(const std::experimental::source_location &l) const
      {
        throw error::Error("Get wrong type.[Required T = '"
                           + details::get_typename(details::index_of_v<T, details::VTList>)
                           + "', Value T = '" + details::get_typename(value.index())
                           + "'], Requires from " + error::location_to_str(l));
      }
    };
  }
}
#endif