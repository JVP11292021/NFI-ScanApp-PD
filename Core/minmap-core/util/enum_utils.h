#pragma once

#include "logging.h"

#include <iostream>
#include <memory>
#include <array>
#include <string_view>
#include <stdexcept>

//#include <boost/preprocessor.hpp>

namespace colmap {
    namespace enum_util {

        constexpr bool is_space(char c) { return c == ' ' || c == '\t' || c == '\n'; }

        constexpr std::string_view trim(std::string_view s) {
            std::size_t b = 0, e = s.size();
            while (b < e && is_space(s[b])) ++b;
            while (e > b && is_space(s[e - 1])) --e;
            return s.substr(b, e - b);
        }

        template <std::size_t N>
        constexpr auto split(std::string_view s) {
            std::array<std::string_view, N> out{};
            size_t idx = 0, start = 0;

            for (size_t i = 0; i <= s.size(); ++i) {
                if (i == s.size() || s[i] == ',') {
                    out[idx++] = trim(s.substr(start, i - start));
                    start = i + 1;
                }
            }
            return out;
        }

    } // namespace enum_util

#define MAKE_ENUM_CLASS(name, start, ...)                                      \
enum class name { __VA_ARGS__ };                                                \
                                                                               \
constexpr auto name##_names =                                                  \
    enum_util::split<name##_count>(#__VA_ARGS__);                              \
                                                                               \
constexpr std::string_view name##ToString(name v) {                            \
    int i = int(v) - start;                                                    \
    if (i < 0 || i >= int(name##_names.size()))                                \
        throw std::runtime_error("Bad enum value");                            \
    return name##_names[i];                                                    \
}                                                                              \
                                                                               \
constexpr name name##FromString(std::string_view s) {                          \
    for (size_t i = 0; i < name##_names.size(); ++i)                           \
        if (name##_names[i] == s)                                              \
            return name(i + start);                                            \
    throw std::runtime_error("Bad enum string");                               \
}

#define ENUM_PP_NARG(...) ENUM_PP_NARG_(__VA_ARGS__, ENUM_PP_RSEQ_N())
#define ENUM_PP_NARG_(...) ENUM_PP_ARG_N(__VA_ARGS__)
#define ENUM_PP_ARG_N( \
     _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, \
     _17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32, \
     _33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48, \
     _49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64, \
     N, ...) N
#define ENUM_PP_RSEQ_N() \
     64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41, \
     40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17, \
     16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define ENUM_COUNT(...) ENUM_PP_NARG(__VA_ARGS__)

#define MAKE_ENUM_CLASS(name, start, ...)                         \
constexpr size_t name##_count = ENUM_COUNT(__VA_ARGS__);         \
enum class name { __VA_ARGS__ };                                  \
constexpr auto name##_names =                                     \
    enum_util::split<name##_count>(#__VA_ARGS__);                 \
constexpr std::string_view name##ToString(name v) {               \
    int i = int(v) - start;                                       \
    if (i < 0 || i >= int(name##_names.size()))                   \
        throw std::runtime_error("Bad enum value");               \
    return name##_names[i];                                       \
}                                                                 \
constexpr name name##FromString(std::string_view s) {             \
    for (size_t i = 0; i < name##_names.size(); ++i)              \
        if (name##_names[i] == s)                                 \
            return name(i + start);                               \
    throw std::runtime_error("Bad enum string");                  \
}

#define MAKE_ENUM_CLASS_STREAM(name, start, ...)                  \
MAKE_ENUM_CLASS(name, start, __VA_ARGS__)                          \
inline std::ostream& operator<<(std::ostream& os, name v) {        \
    return os << name##ToString(v);                                 \
}

}  // namespace colmap


//#include "logging.h"
//
//#include <iostream>
//#include <memory>
//
////#include <boost/preprocessor.hpp>
//
//namespace colmap {
//
//// Custom macro for enum to/from string support. Only enum structs / classes
//// with consecutive indexes are supported.
////
//// Example:
//// [Reference]: enum class MyEnum {C1, C2, C3};
//// [New code]: MAKE_ENUM_CLASS(MyEnum, 0, C1, C2, C3);
////             MyEnumToString(MyEnum::C1);  -> "C1"
////             MyEnumFromString("C2");      -> MyEnum::C2
//
//#define ENUM_TO_STRING_PROCESS_ELEMENT(r, start_idx, idx, elem) \
//  case ((idx) + (start_idx)):                                   \
//    return BOOST_PP_STRINGIZE(elem);
//#define ENUM_FROM_STRING_PROCESS_ELEMENT(r, name, idx, elem) \
//  if (str == BOOST_PP_STRINGIZE(elem)) {                     \
//    return name::elem;                                       \
//  }
//
//#define DEFINE_ENUM_TO_FROM_STRING(name, start_idx, ...)                     \
//  [[maybe_unused]] static std::string_view name##ToString(int value) {       \
//    switch (value) {                                                         \
//      BOOST_PP_SEQ_FOR_EACH_I(ENUM_TO_STRING_PROCESS_ELEMENT,                \
//                              start_idx,                                     \
//                              BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__));        \
//      default:                                                               \
//        throw std::runtime_error("Unknown value: " + std::to_string(value) + \
//                                 " for enum: " + BOOST_PP_STRINGIZE(name));  \
//    }                                                                        \
//  }                                                                          \
//  [[maybe_unused]] static std::string_view name##ToString(name value) {      \
//    return name##ToString(static_cast<int>(value));                          \
//  }                                                                          \
//  [[maybe_unused]] static name name##FromString(std::string_view str) {      \
//    BOOST_PP_SEQ_FOR_EACH_I(ENUM_FROM_STRING_PROCESS_ELEMENT,                \
//                            name,                                            \
//                            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__));          \
//    throw std::runtime_error("Unknown string value: " + std::string(str) +   \
//                             " for enum: " + BOOST_PP_STRINGIZE(name));      \
//  }
//
//#define ENUM_PROCESS_ELEMENT(r, start_idx, idx, elem) \
//  elem = (idx) + (start_idx),
//
//#define ENUM_VALUES(start_idx, ...) \
//  BOOST_PP_SEQ_FOR_EACH_I(          \
//      ENUM_PROCESS_ELEMENT, start_idx, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
//
//#define MAKE_ENUM(name, start_idx, ...)              \
//  enum name { ENUM_VALUES(start_idx, __VA_ARGS__) }; \
//  DEFINE_ENUM_TO_FROM_STRING(name, start_idx, __VA_ARGS__)
//
//#define MAKE_ENUM_CLASS(name, start_idx, ...)              \
//  enum class name { ENUM_VALUES(start_idx, __VA_ARGS__) }; \
//  DEFINE_ENUM_TO_FROM_STRING(name, start_idx, __VA_ARGS__)
//
//// This only works for non-nested enum classes.
//#define MAKE_ENUM_CLASS_OVERLOAD_STREAM(name, start_idx, ...)     \
//  MAKE_ENUM_CLASS(name, start_idx, __VA_ARGS__);                  \
//  inline std::ostream& operator<<(std::ostream& os, name value) { \
//    return os << name##ToString(value);                           \
//  }
//
//}  // namespace colmap
