#ifndef ARTICLE_ANALYSIS_PROTOCOL_NET_H_
#define ARTICLE_ANALYSIS_PROTOCOL_NET_H_


#include <stdint.h>
#include <string.h>

#include <array>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "logging/logger.h"
#include "protocol/i_dumpable.h"
#include "protocol/i_loadable.h"


#pragma pack(push, 1)


namespace protocol {

namespace net {


/**
 * Enuemrates of the type of packages.
 */
enum class PackageType : uint32_t {
  ASYNC_BIT = 0x100000,
  REPLY_QUERY_BIT = 0x01,

  QUERY_MAX_ID = 0x100,
  REPLY_MAX_ID = QUERY_MAX_ID | REPLY_QUERY_BIT,

  QUERY_DOC_META_DATA_AFTER_ID = 0x200,
  REPLY_DOC_META_DATA_AFTER_ID = QUERY_DOC_META_DATA_AFTER_ID | REPLY_QUERY_BIT,

  QUERY_DOC_META_DATA_AFTER_TIME = 0x300,
  REPLY_DOC_META_DATA_AFTER_TIME =
      QUERY_DOC_META_DATA_AFTER_TIME | REPLY_QUERY_BIT,

  QUERY_DOC_META_DATA_OF_AUTHOR = 0x400,
  REPLY_DOC_META_DATA_OF_AUTHOR =
      QUERY_DOC_META_DATA_OF_AUTHOR  | REPLY_QUERY_BIT,

  QUERY_DOC_META_DATA_SERIES = 0x500,
  REPLY_DOC_META_DATA_SERIES = QUERY_DOC_META_DATA_SERIES | REPLY_QUERY_BIT,

  QUERY_DOC_REAL_DATA = 0x600,
  REPLY_DOC_REAL_DATA = QUERY_DOC_REAL_DATA | REPLY_QUERY_BIT,

  REGISTER_NEW_DOC_LISTENER = 0x700 | ASYNC_BIT,
  UNREGISTER_NEW_DOC_LISTENER = 0x710 | ASYNC_BIT,
  NOTIFY_NEW_DOC_LISTENER = 0x720,

  REGISTER_DOC_META_DATA_CHANGED_LISTENER = 0x800 | ASYNC_BIT,
  UNREGISTER_DOC_META_DATA_CHANGED_LISTENER = 0x810 | ASYNC_BIT,
  NOTIFY_DOC_META_DATA_CHANGED_LISTENER = 0x820,

  QUERY_URL_BY_ID = 0x900,
  REPLY_URL_BY_ID = QUERY_URL_BY_ID | REPLY_QUERY_BIT,

  QUERY_ID_BY_URL = 0xa00,
  REPLY_ID_BY_URL = QUERY_ID_BY_URL | REPLY_QUERY_BIT
};


struct PackageHeader {
  PackageType type;
  uint64_t size;

  PackageHeader() {}

  PackageHeader(PackageType type, uint64_t size) : type(type), size(size) {}
};


namespace internal_ {


void InitIfFirst();


template <bool IS_DERIVED_FROM_IDUMPABLE>
struct Converter {
  template <typename Type>
  static std::string Dump(Type const& value) {
    return std::string(reinterpret_cast<char const*>(&value), sizeof(value));
  }

  template <typename Type>
  static bool Load(std::string const& buf, size_t* offset, Type* result) {
    if (*offset + sizeof(Type) > buf.length()) {
      return false;
    }
    memcpy(reinterpret_cast<void*>(result),
           reinterpret_cast<void const*>(buf.data() + *offset),
           sizeof(Type));
    *offset += sizeof(Type);
    return true;
  }
};


template <>
struct Converter<true> {
  template <typename Type>
  static std::string Dump(Type const& value) {
    return value.Dump();
  }

  template <typename Type>
  static bool Load(std::string const& buf, size_t* offset, Type* result) {
    return result->Load(buf, offset);
  }
};


}  // namespace internal_


/**
 * A converter for specific type instances.
 */
template <typename Type>
struct Converter {
  /**
   * Dumps the content of the value into bytes data.
   *
   * @param [in] value Value to be dump.
   * @reutrn A string.
   */
  static std::string Dump(Type const& value) {
    typedef internal_::Converter<std::is_base_of<IDumpable, Type>::value> C;
    return C::Dump(value);
  }

  /**
   * Loads the content of the value from bytes data.
   *
   * @param [in] buf The bytes data.
   * @param [in,out] offset Offset of the real value in the bytes data.
   *     After the value loaded, it will be move forward to skip the loaded
   *     data.
   * @param [out] Address of the result to be put.
   * @return `true` if no error occured.
   */
  static bool Load(std::string const& buf, size_t* offset, Type* result) {
    typedef internal_::Converter<std::is_base_of<ILoadable, Type>::value> C;
    return C::Load(buf, offset, result);
  }
};


template <>
struct Converter<std::string> {
  static std::string Dump(std::string const& value) {
    return Converter<uint64_t>::Dump(value.size()) + value;
  }

  static bool Load(std::string const& buf, size_t* offset,
                   std::string* result) {
    uint64_t size;
    if (!Converter<uint64_t>::Load(buf, offset, &size)) {
      return false;
    }

    if (*offset + size > buf.length()) {
      return false;
    }

    *result = std::string(buf.data() + *offset, size);
    *offset += size;
    return true;
  }
};


template <>
struct Converter<std::wstring> {
  static std::string Dump(std::wstring const& value) {
    internal_::InitIfFirst();

    char buf[MB_CUR_MAX];

    std::string s;
    { int tmp; tmp = wctomb(NULL, 0); ++tmp; }
    for (wchar_t const& c : value) {
      int delta = wctomb(buf, c);
      if (delta < 1) {
        logging::GetRootLogger()->Fatal(
            "Error while dumping the std::wstring `%ls`.", value);
      }
      for (int i = 0; i < delta; ++i) {
        s += buf[i];
      }
    }

    return Converter<uint64_t>::Dump(s.length()) + s;
  }

  static bool Load(std::string const& buf, size_t* offset,
                   std::wstring* result) {
    internal_::InitIfFirst();

    uint64_t size;
    if (!Converter<uint64_t>::Load(buf, offset, &size)) {
      return false;
    }

    if (*offset + size > buf.length()) {
      return false;
    }

    mblen(NULL, 0);
    mbtowc(NULL, NULL, 0);

    *result = L"";
    for (char const* ptr = buf.data() + *offset; size > 0; ) {
      int len = mblen(ptr, size);
      if (len < 1) {
        logging::GetRootLogger()->Fatal("Decode error on std::wstring!.");
      }

      wchar_t tmp;
      mbtowc(&tmp, ptr, len);
      *result += tmp;

      ptr += len;
      size -= len;
      *offset += len;
    }
    return true;
  }
};


template <typename T>
struct Converter<std::vector<T>> {
  static std::string Dump(std::vector<T> const& v) {
    std::string ret;
    ret += Converter<uint64_t>::Dump(v.size());
    for (auto& it : v) {
      ret += Converter<T>::Dump(it);
    }
    return ret;
  }

  static bool Load(std::string const& buf, size_t* offset,
                   std::vector<T>* result) {
    uint64_t size;
    if (!Converter<uint64_t>::Load(buf, offset, &size)) {
      return false;
    }
    result->resize(size);
    for (size_t i = 0; i < size; ++i) {
      if (!Converter<T>::Load(buf, offset, &result->at(i))) {
        return false;
      }
    }
    return true;
  }
};


template <typename T, size_t SZ>
struct Converter<std::array<T, SZ>> {
  static std::string Dump(std::array<T, SZ> const& arr) {
    std::string s;
    for (size_t i = 0; i < SZ; ++i) {
      s += Converter<T>::Dump(arr[i]);
    }
    return s;
  }

  static bool Load(std::string const& buf, size_t* offset,
                   std::array<T, SZ>* arr) {
    for (size_t i = 0; i < SZ; ++i) {
      if (!Converter<T>::Load(buf, offset, &arr->at(i))) {
        return false;
      }
    }
    return true;
  }
};


/**
 * @see Converter::Dump
 */
template <typename Type>
std::string Dump(Type const& value) {
  return Converter<Type>::Dump(value);
}

template <typename Type0, typename Type1, typename... Args>
std::string Dump(Type0 const& value0, Type1 const& value1, Args&& ...args) {
  return Dump(value0) + Dump(value1, std::forward<Args>(args)...);
}


/**
 * @see Converter::Load
 */
template <typename Type>
bool Load(std::string const& buf, size_t* offset, Type* value) {
  return Converter<Type>::Load(buf, offset, value);
}


template <typename Type0, typename Type1, typename... Args>
bool Load(std::string const& buf, size_t* offset,
          Type0* value0, Type1* value1, Args&& ...args) {
  bool ret = true;
  ret = ret && Load(buf, offset, value0);
  ret = ret && Load(buf, offset, value1, std::forward<Args>(args)...);
  return ret;
}

}  // namespace net

}  // namespace protocol


#pragma pack(pop)

#endif  // ARTICLE_ANALYSIS_PROTOCOL_NET_H_
