#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "utils/types.h"
#include "utils/variant.h"


namespace {


TEST(Variant, IsTypeOf_IsEmpty_Clear) {
  utils::Variant a;

  EXPECT_TRUE(a.IsTypeOf<void>());
  EXPECT_TRUE(a.IsEmpty());

  a.set_value(7);
  EXPECT_TRUE(a.IsTypeOf<int>());
  EXPECT_FALSE(a.IsTypeOf<float>());

  a.set_value((char const*)"abc");
  EXPECT_TRUE(a.IsTypeOf<char const*>());
  EXPECT_FALSE(a.IsTypeOf<float>());

  a.set_value(std::string("abc"));
  EXPECT_TRUE(a.IsTypeOf<std::string>());
  EXPECT_FALSE(a.IsTypeOf<float>());

  a.Clear();
  EXPECT_TRUE(a.IsTypeOf<void>());
  EXPECT_TRUE(a.IsEmpty());
}


template <typename T>
utils::Variant SimpleFunc(T&& value) {
  utils::Variant ret;
  ret.set_value<utils::RemoveRef<T>>(std::forward<T>(value));
  return ret;
}


TEST(Variant, CopyFrom_Swap) {
  utils::Variant a, b;

  a.set_value(7);
  b.set_value(8);
  int* ptr_a = a.address<int>();
  int* ptr_b = b.address<int>();

  a.Swap(b);
  EXPECT_EQ(a.value<int>(), 8);
  EXPECT_EQ(b.value<int>(), 7);
  EXPECT_EQ(a.address<int>(), ptr_b);
  EXPECT_EQ(b.address<int>(), ptr_a);

  std::swap(a, b);
  EXPECT_EQ(a.value<int>(), 7);
  EXPECT_EQ(b.value<int>(), 8);
  EXPECT_EQ(a.address<int>(), ptr_a);
  EXPECT_EQ(b.address<int>(), ptr_b);

  a.Swap(utils::Variant());
  EXPECT_TRUE(a.IsTypeOf<void>() && a.IsEmpty());

  b.Swap(SimpleFunc(std::string("123")));
  EXPECT_TRUE(b.IsTypeOf<std::string>() && b.value<std::string>() == "123");
}


TEST(Variant, value_and_address) {
  utils::Variant a;

  a.set_value(5);
  EXPECT_EQ(a.value<int>(), 5);

  int x = 7;
  a.set_value(x);
  EXPECT_EQ(a.value<int>(), 7);

  int* ptr1 = a.address<int>();
  a.value<int>() = 9;
  EXPECT_EQ(a.value<int>(), 9);
  EXPECT_EQ(a.address<int>(), ptr1);
  EXPECT_EQ(a.address<void>(), reinterpret_cast<void*>(ptr1));
}

}  // namespace
