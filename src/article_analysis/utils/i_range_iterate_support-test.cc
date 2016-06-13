#include <stdlib.h>

#include <vector>

#include <gtest/gtest.h>

#include "utils/i_range_iterate_support.h"


namespace {


class MyIterS : public utils::IRangeIterateSupport<int*, int const*> {
 public:
  MyIterS(int* begin, int* end) : begin_(begin), end_(end) {}

  int* begin() override final { return begin_; }
  int* end() override final { return end_; }
  int const* begin() const override final { return begin_; }
  int const* end() const override final { return end_; }

 private:
  int* begin_;
  int* end_;
};


TEST(MyIterS, Iterate) {
  std::vector<int> ref;

  for (int i = 0; i < 10; ++i) {
    ref.push_back(rand());
  }

  MyIterS iter_s(&ref[0], &ref.back() + 1);
  {
    int i = 0;
    for (int a : iter_s) {
      EXPECT_EQ(a, ref[i]);
      ++i;
    }
  }

  {
    int i = 0;
    for (int &a : iter_s) {
      a = rand();
      EXPECT_EQ(a, ref[i]);
      ++i;
    }
  }

  {
    int i = 0;
    for (int a : static_cast<MyIterS const>(iter_s)) {
      EXPECT_EQ(a, ref[i]);
      ++i;
    }
  }
}

}  // namespace
