// TODO(b01902109<at>ntu.edu.tw): Implement to endian-safe converters.

#ifndef ARTICLE_ANALYSIS_PROTOCOL_CONVERT_H_
#define ARTICLE_ANALYSIS_PROTOCOL_CONVERT_H_


#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <utility>

#include "logger/logger.h"


namespace protocol {

namespace converter {

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
    return std::string(reinterpret_cast<char const*>(&value), sizeof(value));
  }

  /**
   * Loads the content of the value from bytes data.
   *
   * @param [in] buf The bytes data.
   * @param [in,out] offset Offset of the real value in the bytes data.
   *     After the value loaded, it will be move forward to skip the loaded
   *     data.
   * @return The loaded value.
   */
  static Type Load(std::string const& buf, size_t* offset) {
    Type ret(*reinterpret_cast<Type const*>(buf.data() + *offset));
    *offset += sizeof(Type);
    return ret;
  }


  /**
   * Loads the content of the value from bytes data.
   *
   * @param [out] Address of the result to be put.
   * @see Load()
   */
  static void LoadTo(std::string const& buf, size_t* offset, Type* result) {
    memcpy(reinterpret_cast<void*>(result),
           reinterpret_cast<void const*>(buf.data() + *offset),
           sizeof(Type));
    *offset += sizeof(Type);
  }
};


template <>
struct Converter<std::string> {
  static std::string Dump(std::string const& value) {
    return Converter<size_t>::Dump(value.size()) + value;
  }

  static std::string Load(std::string const& buf, size_t* offset) {
    size_t size = Converter<size_t>::Load(buf, offset);

    std::string ret(buf.data() + *offset, size);
    *offset += size;

    return ret;
  }

  static void LoadTo(std::string const& buf, size_t* offset,
                     std::string* result) {
    *result = Load(buf, offset);
  }
};


template <>
struct Converter<std::wstring> {
  static std::string Dump(std::wstring const& value) {
    size_t size = 0;

    char buf[MB_CUR_MAX];

    { int tmp; tmp = wctomb(NULL, 0); ++tmp; }
    for (wchar_t const& c : value) {
      int delta = wctomb(buf, c);
      if (delta < 1) {
        logger::Fatal("Error while dumping the std::wstring `%ls`.", value);
      }
      size += delta;
    }

    std::string s;
    { int tmp; tmp = wctomb(NULL, 0); ++tmp; }
    for (wchar_t const& c : value) {
      int delta = wctomb(buf, c);
      if (delta < 1) {
        logger::Fatal("Error while dumping the std::wstring `%ls`.", value);
      }
      for (int i = 0; i < delta; ++i) {
        s += buf[i];
      }
    }

    return Converter<size_t>::Dump(size) + s;
  }

  static std::wstring Load(std::string const& buf, size_t *offset) {
    size_t size = Converter<size_t>::Load(buf, offset);

    mblen(NULL, 0);
    mbtowc(NULL, NULL, 0);

    std::wstring ret;
    for (char const* ptr = buf.data() + *offset; size > 0; ) {
      int len = mblen(ptr, size);
      if (len < 1) {
        logger::Fatal("Decode error on std::wstring!.");
      }

      wchar_t tmp;
      mbtowc(&tmp, ptr, len);
      ret += tmp;

      ptr += len;
      size -= len;
      *offset += len;
    }

    return ret;
  }

  static void LoadTo(std::string const& buf, size_t* offset,
                     std::wstring* result) {
    *result = Load(buf, offset);
  }
};


template <typename T>
struct Converter<std::vector<T>> {
  static std::string Dump(std::vector<T> const& v) {
    std::string ret;
    ret += Converter<size_t>::Dump(v.size());
    for (auto& it : v) {
      ret += Converter<T>::Dump(it);
    }
    return ret;
  }

  static std::vector<T> Load(std::string const& buf, size_t* offset) {
    size_t size = Converter<size_t>::Load(buf, offset);
    std::vector<T> ret;
    for (size_t i = 0; i < size; ++i) {
      ret.emplace_back(Converter<T>::Load(buf, offset));
    }
    return ret;
  }

  static void LoadTo(std::string const& buf, size_t* offset,
                     std::vector<T>* result) {
    *result = Load(buf, offset);
  }
};


void Init() {
  setlocale(LC_ALL, "");
}


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
 * @see Converter::LoadTo
 */
template <typename Type>
void LoadTo(std::string const& buf, size_t* offset, Type* value) {
  return Converter<Type>::LoadTo(buf, offset, value);
}

template <typename Type0, typename Type1, typename... Args>
void LoadTo(std::string const& buf, size_t* offset,
            Type0* value0, Type1* value1, Args&& ...args) {
  LoadTo(buf, offset, value0);
  LoadTo(buf, offset, value1, std::forward<Args>(args)...);
}

}  // namespace converter

}  // namespace protocol

#endif  // ARTICLE_ANALYSIS_PROTOCOL_CONVERT_H_
