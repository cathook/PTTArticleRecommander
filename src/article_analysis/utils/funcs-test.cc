#include <string>

#include <gtest/gtest.h>

#include "utils/funcs.h"


using std::string;


namespace {


TEST(FormatStr, IsCorrect) {
  int a = 7;
  std::string s("def");
  EXPECT_EQ(utils::FormatStr("%d %d %s %s %s",
                             a, 8, "abc", s, std::string("ghi")),
            "7 8 abc def ghi");
}


TEST(TransformFromStr, int) {
  string a = "123 ";
  EXPECT_EQ(utils::TransformFromStr<int>(a), 123);
  EXPECT_EQ(utils::TransformFromStr<int>(move(a)), 123);
}

TEST(TransformFromStr, double) {
  string b = "123.4";
  EXPECT_DOUBLE_EQ(utils::TransformFromStr<double>(b), 123.4);
  EXPECT_DOUBLE_EQ(utils::TransformFromStr<double>(move(b)), 123.4);
}

TEST(TransformFromStr, string) {
  string c = "meow meow";
  EXPECT_EQ(utils::TransformFromStr<string>(c), "meow meow");
  EXPECT_EQ(utils::TransformFromStr<string>(move(c)), "meow meow");
}

}  // namespace
