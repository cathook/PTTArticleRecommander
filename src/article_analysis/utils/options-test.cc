#include <stdlib.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "utils/options.h"


using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;


namespace {


TEST(TypedOption, int) {
  utils::TypedOption<int> opt(7, "meow meow");
  EXPECT_EQ(opt.value(), 7);
  EXPECT_EQ(opt.AAnOption::value<int>(), 7);
  EXPECT_EQ(opt.desc(), "meow meow");

  opt.set_value(9);
  EXPECT_EQ(opt.value(), 9);
  EXPECT_EQ(opt.AAnOption::value<int>(), 9);
  EXPECT_EQ(opt.desc(), "meow meow");

  opt.AAnOption::set_value<int>(11);
  EXPECT_EQ(opt.value(), 11);
  EXPECT_EQ(opt.AAnOption::value<int>(), 11);
  EXPECT_EQ(opt.desc(), "meow meow");

  opt.AAnOption::set_value<string>("18");
  EXPECT_EQ(opt.value(), 18);
  EXPECT_EQ(opt.AAnOption::value<int>(), 18);
  EXPECT_EQ(opt.desc(), "meow meow");
}


TEST(TypedOption, string) {
  utils::TypedOption<string> opt("meow meow", "meow meow");
  EXPECT_EQ(opt.value(), "meow meow");
  EXPECT_EQ(opt.AAnOption::value<string>(), "meow meow");
  EXPECT_EQ(opt.desc(), "meow meow");

  opt.set_value("wang wang");
  EXPECT_EQ(opt.value(), "wang wang");
  EXPECT_EQ(opt.AAnOption::value<string>(), "wang wang");
  EXPECT_EQ(opt.desc(), "meow meow");

  opt.AAnOption::set_value<string>("wu lala");
  EXPECT_EQ(opt.value(), "wu lala");
  EXPECT_EQ(opt.AAnOption::value<string>(), "wu lala");
  EXPECT_EQ(opt.desc(), "meow meow");

  opt.AAnOption::set_value<string>("ha ha ha");
  EXPECT_EQ(opt.value(), "ha ha ha");
  EXPECT_EQ(opt.AAnOption::value<string>(), "ha ha ha");
  EXPECT_EQ(opt.desc(), "meow meow");
}


class MyOptionCollectionA : public utils::AOptionCollection {
 public:
  MyOptionCollectionA() : AOptionCollection("A") {}
};

TEST(MyOptionCollectionA, All) {
  MyOptionCollectionA a;

  EXPECT_EQ(a.desc(), "A");

  {
    size_t counter = 0;
    for (auto& it : a.iterators()) {
      it.name();
      ++counter;
    }
    EXPECT_EQ(counter, 0);
  }

  {
    size_t counter = 0;
    for (auto& it : a.an_option_only_iterators()) {
      it.name();
      ++counter;
    }
    EXPECT_EQ(counter, 0);
  }
}


class MyOptionCollectionB : public utils::AOptionCollection {
 public:
  MyOptionCollectionB() : AOptionCollection("B") {
    AddOption<utils::TypedOption<int>>("int1", 1, "int1");
    AddOption<utils::TypedOption<int>>("int2", 2, "int2");
    AddOption<utils::TypedOption<int>>("int3", 3, "int3");
  }
};

TEST(MYOptionCollectionB, All) {
  MyOptionCollectionB b;
  EXPECT_EQ(b.GetOption<utils::TypedOption<int>>("int1")->value(), 1);
  EXPECT_EQ(b.GetOption<utils::TypedOption<int>>("int2")->value(), 2);
  EXPECT_EQ(b.GetOption<utils::TypedOption<int>>("int3")->value(), 3);

  unordered_set<string> attrs;
  attrs.insert("int1");
  attrs.insert("int2");
  attrs.insert("int3");

  vector<MyOptionCollectionB::IteratorValueType> iters;
  for (auto it : b.iterators()) {
    iters.emplace_back(it);

    EXPECT_TRUE(attrs.count(it.name()) > 0);
    EXPECT_EQ(it.name(), it.desc());

    attrs.erase(it.name());
  }

  EXPECT_TRUE(attrs.empty());

  {
    size_t i = 0;
    for (auto it : b.iterators()) {
      ASSERT_TRUE(i < iters.size());
      EXPECT_TRUE(it == iters[i]);
      ++i;
    }
  }
}


class MyOptionCollectionC : public utils::AOptionCollection {
 public:
  MyOptionCollectionC() : AOptionCollection("C") {
    AddOption<MyOptionCollectionA>("A1");
    AddOption<MyOptionCollectionB>("B1");
    AddOption<MyOptionCollectionB>("B2");
  }
};

TEST(MyOptionCollectionC, All) {
  MyOptionCollectionC c;

  {
    unordered_map<string, string> name_desc;
    name_desc.emplace("A1", "A");
    name_desc.emplace("B1", "B");
    name_desc.emplace("B1.int1", "int1");
    name_desc.emplace("B1.int2", "int2");
    name_desc.emplace("B1.int3", "int3");
    name_desc.emplace("B2", "B");
    name_desc.emplace("B2.int1", "int1");
    name_desc.emplace("B2.int2", "int2");
    name_desc.emplace("B2.int3", "int3");

    for (auto it : c.iterators()) {
      auto it2 = name_desc.find(it.name());

      EXPECT_TRUE(it2 != name_desc.end());
      EXPECT_EQ(it2->second, it.desc());

      name_desc.erase(it.name());
    }
    EXPECT_TRUE(name_desc.empty());
  }

  {
    unordered_map<string, string> name_desc;
    name_desc.emplace("B1.int1", "int1");
    name_desc.emplace("B1.int2", "int2");
    name_desc.emplace("B1.int3", "int3");
    name_desc.emplace("B2.int1", "int1");
    name_desc.emplace("B2.int2", "int2");
    name_desc.emplace("B2.int3", "int3");

    for (auto it : c.an_option_only_iterators()) {
      auto it2 = name_desc.find(it.name());

      EXPECT_TRUE(it2 != name_desc.end());
      EXPECT_EQ(it2->second, it.desc());

      name_desc.erase(it.name());
    }
    EXPECT_TRUE(name_desc.empty());
  }
}


class MyOptionCollectionD : public utils::AOptionCollection {
 public:
  MyOptionCollectionD() : AOptionCollection("D") {
    AddOption<utils::TypedOption<string>>("string1", "string1", "string1");
    AddOption<utils::TypedOption<string>>("string2", "string2", "string2");
    AddOption<utils::TypedOption<string>>("string3", "string3", "string3");
    AddOption<MyOptionCollectionC>("C1");
    AddOption<MyOptionCollectionC>("C2");
  }
};

TEST(MyOptionCollectionD, All) {
  MyOptionCollectionD d;

  {
    unordered_map<string, string> name_desc;
    name_desc.emplace("string1", "string1");
    name_desc.emplace("string2", "string2");
    name_desc.emplace("string3", "string3");
    name_desc.emplace("C1", "C");
    name_desc.emplace("C1.A1", "A");
    name_desc.emplace("C1.B1", "B");
    name_desc.emplace("C1.B1.int1", "int1");
    name_desc.emplace("C1.B1.int2", "int2");
    name_desc.emplace("C1.B1.int3", "int3");
    name_desc.emplace("C1.B2", "B");
    name_desc.emplace("C1.B2.int1", "int1");
    name_desc.emplace("C1.B2.int2", "int2");
    name_desc.emplace("C1.B2.int3", "int3");
    name_desc.emplace("C2", "C");
    name_desc.emplace("C2.A1", "A");
    name_desc.emplace("C2.B1", "B");
    name_desc.emplace("C2.B1.int1", "int1");
    name_desc.emplace("C2.B1.int2", "int2");
    name_desc.emplace("C2.B1.int3", "int3");
    name_desc.emplace("C2.B2", "B");
    name_desc.emplace("C2.B2.int1", "int1");
    name_desc.emplace("C2.B2.int2", "int2");
    name_desc.emplace("C2.B2.int3", "int3");

    for (auto it : d.iterators()) {
      auto it2 = name_desc.find(it.name());

      EXPECT_TRUE(it2 != name_desc.end());
      EXPECT_EQ(it2->second, it.desc());

      name_desc.erase(it.name());
    }
    EXPECT_TRUE(name_desc.empty());
  }

  {
    unordered_map<string, string> name_desc;
    name_desc.emplace("string1", "string1");
    name_desc.emplace("string2", "string2");
    name_desc.emplace("string3", "string3");
    name_desc.emplace("C1.B1.int1", "int1");
    name_desc.emplace("C1.B1.int2", "int2");
    name_desc.emplace("C1.B1.int3", "int3");
    name_desc.emplace("C1.B2.int1", "int1");
    name_desc.emplace("C1.B2.int2", "int2");
    name_desc.emplace("C1.B2.int3", "int3");
    name_desc.emplace("C2.B1.int1", "int1");
    name_desc.emplace("C2.B1.int2", "int2");
    name_desc.emplace("C2.B1.int3", "int3");
    name_desc.emplace("C2.B2.int1", "int1");
    name_desc.emplace("C2.B2.int2", "int2");
    name_desc.emplace("C2.B2.int3", "int3");

    for (auto it : d.an_option_only_iterators()) {
      auto it2 = name_desc.find(it.name());

      EXPECT_TRUE(it2 != name_desc.end());
      EXPECT_EQ(it2->second, it.desc());
      if (it.name().find("string") == 0) {
        EXPECT_EQ(d.GetOption<utils::TypedOption<string>>(it.name())->value(),
                  it.desc());
      } else if (it.name().find("int") == 0) {
        int val = atoi(it.name().substr(3).c_str());
        EXPECT_EQ(d.GetOption<utils::TypedOption<int>>(it.name())->value(),
                  val);
      }

      name_desc.erase(it.name());
    }
    EXPECT_TRUE(name_desc.empty());
  }
}

}  // namespace
