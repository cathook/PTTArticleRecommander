#include <stdlib.h>

#include <gtest/gtest.h>

#include "arg_parser/arg_parser.h"


namespace {


TEST(ArgParser, Usage_Desc) {
  arg_parser::ArgParser ap;
  ap.AddFlag("flag1", "flag1_desc", []() { return true; });
  ap.AddFlag("flag2", "flag2_desc", [](bool k) { return true; });
  ap.AddOpt("opt1", "opt1_value", "opt1_desc",
            [](std::string const& s) { return true; });
  ap.AddOptionalOpt("opt2", "opt2_value", "opt2_desc",
                    [](std::string const& s) { return true; },
                    "meow");

  EXPECT_EQ(ap.GetArgsUsage(),
            "[flag1] [flag2] opt1 <opt1_value> [opt2 <opt2_value>]");

  EXPECT_EQ(ap.GetArgsDesc(4),
            "    flag1\tflag1_desc\n"
            "    flag2\tflag2_desc\n"
            "    opt1 <opt1_value>\topt1_desc\n"
            "    opt2 <opt2_value>\t(optional, default=`meow`) opt2_desc\n");
}


TEST(ArgParser, Parse) {
  bool a = false;
  bool x = false;
  int b = 0;
  double c = 0;
  double d = 0;
  double e = 0;
  double f = 0;

  auto ToDbl = [](std::string const& s) { return atof(s.c_str()); };

  arg_parser::ArgParser ap;
  ap.AddFlag("a", "", [&]() { a = true; return true; });
  ap.AddFlag("x", "", [&]() { x = true; return true; });
  ap.AddFlag("b", "", [&](bool k) { b = 1 + int(k); return true; });
  ap.AddOpt("c", "", "",
            [&](std::string const& s) { c = ToDbl(s); return true; });
  ap.AddOptionalOpt("d", "", "",
                    [&](std::string const& s) { d = ToDbl(s); return true; },
                    "1");
  ap.AddOptionalOpt("e", "", "",
                    [&](std::string const& s) { e = ToDbl(s); return true; },
                    "1", false);
  ap.AddOptionalOpt("f", "", "",
                    [&](std::string const& s) { f = ToDbl(s); return true; },
                    "1", false);

  char const* ptr[] = {
    "prog", "a", "c", "1.2", "--", "meow", "wang", "f", "1.5"
  };
  int size = sizeof(ptr) / sizeof(ptr[0]);

  bool ret = ap.Parse(&size, ptr);

  EXPECT_TRUE(ret);

  EXPECT_EQ(a, true);
  EXPECT_EQ(x, false);
  EXPECT_EQ(b, 1);
  EXPECT_EQ(c, ToDbl("1.2"));
  EXPECT_EQ(d, ToDbl("1"));
  EXPECT_EQ(e, 0);
  EXPECT_EQ(f, ToDbl("1.5"));

  EXPECT_EQ(size, 3);
  EXPECT_TRUE(strcmp(ptr[0], "prog") == 0);
  EXPECT_TRUE(strcmp(ptr[1], "meow") == 0);
  EXPECT_TRUE(strcmp(ptr[2], "wang") == 0);
}


TEST(ArgParser, ParseFail) {
  arg_parser::ArgParser ap;

  ap.AddFlag("a", "", []() { return false; });
  ap.AddFlag("b", "", [](bool k) { return !k; });

  {
    char const* ptr[] = {"prog", "a"};
    int size = sizeof(ptr) / sizeof(ptr[0]);

    bool ret = ap.Parse(&size, ptr);
    EXPECT_FALSE(ret);
  }

  {
    char const* ptr[] = {"prog", "b"};
    int size = sizeof(ptr) / sizeof(ptr[0]);

    bool ret = ap.Parse(&size, ptr);
    EXPECT_FALSE(ret);
  }

  {
    char const* ptr[] = {"prog"};
    int size = sizeof(ptr) / sizeof(ptr[0]);

    bool ret = ap.Parse(&size, ptr);
    EXPECT_TRUE(ret);
  }

  ap.AddOpt("o", "v", "d", [](std::string const& s) { return false; });
  {
    char const* ptr[] = {"prog"};
    int size = sizeof(ptr) / sizeof(ptr[0]);

    bool ret = ap.Parse(&size, ptr);
    EXPECT_FALSE(ret);
  }

  {
    char const* ptr[] = {"prog", "o", "k"};
    int size = sizeof(ptr) / sizeof(ptr[0]);

    bool ret = ap.Parse(&size, ptr);
    EXPECT_FALSE(ret);
  }
}

}  // namespace
