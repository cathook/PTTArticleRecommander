#include <gtest/gtest.h>

#include <stddef.h>

#include <array>
#include <string>
#include <vector>

#include "protocol/net.h"


using std::array;
using std::string;
using std::wstring;
using std::vector;


namespace {


struct Able : protocol::IDumpable, protocol::ILoadable {
  std::string Dump() const override final {
    return "meow";
  }

  bool Load(std::string const& buf, size_t* offset) override final {
    return buf == "meow";
  }
};


TEST(Converter, uint32_t) {
  protocol::net::Converter<uint32_t> c;

  uint32_t a = 0xdeadbeef;

  char ans_buf[] = {'\xef', '\xbe', '\xad', '\xde'};
  EXPECT_EQ(c.Dump(a), string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));

  size_t offset = 0;
  uint32_t b;
  bool ret = c.Load(c.Dump(a), &offset, &b);
  EXPECT_TRUE(ret);
  EXPECT_EQ(b, 0xdeadbeef);
  EXPECT_EQ(offset, sizeof(uint32_t));

  offset = 0;
  EXPECT_FALSE(c.Load("abc", &offset, &b));
  offset = 5;
  EXPECT_FALSE(c.Load("abcmeow", &offset, &b));
  offset = 0;
  EXPECT_TRUE(c.Load("abcmeow", &offset, &b));
}

TEST(Converter, string) {
  protocol::net::Converter<string> c;

  char ans_buf[] = {
    '\x04', '\x00', '\x00', '\x00',
    '\x00', '\x00', '\x00', '\x00',
    'm', 'e', 'o', 'w'
  };

  EXPECT_EQ(c.Dump("meow"),
            string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));

  size_t offset = 0;
  string t;
  bool ret = c.Load(c.Dump("meow"), &offset, &t);
  EXPECT_TRUE(ret);
  EXPECT_EQ(t, "meow");
  EXPECT_EQ(offset, sizeof(uint64_t) + 4);
}

TEST(Converter, wstring) {
  protocol::net::Converter<wstring> cw;
  protocol::net::Converter<string> cs;

  string s("\xe4\xb8\xad\xe6\x96\x87\xe6\xb8\xac\xe8\xa9\xa6");
  EXPECT_EQ(cw.Dump(L"中文測試"), cs.Dump(s));

  size_t offset = 0;
  wstring t;
  bool ret = cw.Load(cw.Dump(L"中文測試"), &offset, &t);
  EXPECT_TRUE(ret);
  EXPECT_EQ(t, L"中文測試");
  EXPECT_EQ(offset, sizeof(uint64_t) + s.size());
}

TEST(Converter, vector) {
  protocol::net::Converter<vector<int>> c;

  vector<int> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);

  size_t offset = 0;
  vector<int> b;
  bool ret = c.Load(c.Dump(a), &offset, &b);
  EXPECT_TRUE(ret);
  ASSERT_EQ(a.size(), b.size());
  for (size_t i = 0; i < a.size(); ++i) {
    EXPECT_EQ(a[i], b[i]);
  }
}

TEST(Converter, array) {
  protocol::net::Converter<array<uint16_t, 3>> c;

  array<uint16_t, 3> a = {2, 4, 6}, b;

  size_t offset = 0;
  bool ret = c.Load(c.Dump(a), &offset, &b);
  EXPECT_TRUE(ret);
  EXPECT_EQ(offset, 2 * 3);
  EXPECT_EQ(a[0], b[0]);
  EXPECT_EQ(a[1], b[1]);
  EXPECT_EQ(a[2], b[2]);
}

TEST(Coverter, able) {
  protocol::net::Converter<Able> a;

  EXPECT_EQ(a.Dump(Able()), "meow");
  Able able;
  size_t offs;
  EXPECT_TRUE(a.Load("meow", &offs, &able));
  EXPECT_FALSE(a.Load("wang", &offs, &able));
}


TEST(Dump, All) {
  uint32_t a = 0xdeadbeef;
  string s = "meow";
  vector<char> v;
  v.push_back('a');
  v.push_back('b');
  v.push_back('c');

  {
    char ans_buf[] = {'\xef', '\xbe', '\xad', '\xde'};
    EXPECT_EQ(protocol::net::Dump(a),
              string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));
  }

  {
    char ans_buf[] = {
      '\xef', '\xbe', '\xad', '\xde',
      '\x04', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'm', 'e', 'o', 'w'
    };
    EXPECT_EQ(protocol::net::Dump(a, s),
              string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));
  }

  {
    char ans_buf[] = {
      '\xef', '\xbe', '\xad', '\xde',
      '\x04', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'm', 'e', 'o', 'w',
      '\x03', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'a', 'b', 'c'
    };
    EXPECT_EQ(protocol::net::Dump(a, s, v),
              string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));
  }
}


TEST(Load, All) {
  {
    uint32_t a;
    char buf[] = {'\xef', '\xbe', '\xad', '\xde'};
    size_t size = sizeof(buf) / sizeof(buf[0]);
    size_t offset = 0;
    protocol::net::Load(string(buf, size), &offset, &a);
    EXPECT_EQ(a, 0xdeadbeef);
  }

  {
    uint32_t a;
    string s;
    char buf[] = {
      '\xef', '\xbe', '\xad', '\xde',
      '\x04', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'm', 'e', 'o', 'w'
    };
    size_t size = sizeof(buf) / sizeof(buf[0]);
    size_t offset = 0;
    protocol::net::Load(string(buf, size), &offset, &a, &s);
    EXPECT_EQ(a, 0xdeadbeef);
    EXPECT_EQ(s, "meow");
  }

  {
    uint32_t a;
    string s;
    vector<char> v;
    char buf[] = {
      '\xef', '\xbe', '\xad', '\xde',
      '\x04', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'm', 'e', 'o', 'w',
      '\x03', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'a', 'b', 'c'
    };
    size_t size = sizeof(buf) / sizeof(buf[0]);
    size_t offset = 0;
    protocol::net::Load(string(buf, size), &offset, &a, &s, &v);
    EXPECT_EQ(a, 0xdeadbeef);
    EXPECT_EQ(s, "meow");
    EXPECT_EQ(string(&v[0], v.size()), "abc");
  }
}

}  // namespace
