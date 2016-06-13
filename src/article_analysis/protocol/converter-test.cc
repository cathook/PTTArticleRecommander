#include <gtest/gtest.h>

#include <stddef.h>

#include <string>
#include <vector>

#include "protocol/converter.h"


using std::string;
using std::wstring;
using std::vector;


namespace {


TEST(Converter, string) {
  protocol::converter::Converter<string> c;

  char ans_buf[] = {
    '\x04', '\x00', '\x00', '\x00',
    '\x00', '\x00', '\x00', '\x00',
    'm', 'e', 'o', 'w'
  };

  EXPECT_EQ(c.Dump("meow"),
            string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));

  size_t offset = 0;
  EXPECT_EQ(c.Load(c.Dump("meow"), &offset), "meow");
  EXPECT_EQ(offset, sizeof(size_t) + 4);
}

TEST(Converter, wstring) {
  protocol::converter::Init();  // TODO(b01902109<at>ntu.edu.tw): Remove this.

  protocol::converter::Converter<wstring> cw;
  protocol::converter::Converter<string> cs;

  string s("中文測試");
  EXPECT_EQ(cw.Dump(L"中文測試"), cs.Dump(s));

  size_t offset = 0;
  EXPECT_EQ(cw.Load(cw.Dump(L"中文測試"), &offset), L"中文測試");
  EXPECT_EQ(offset, sizeof(size_t) + s.size());
}

TEST(Converter, uint32_t) {
  protocol::converter::Converter<uint32_t> c;

  uint32_t a = 0xdeadbeef;

  char ans_buf[] = {'\xef', '\xbe', '\xad', '\xde'};
  EXPECT_EQ(c.Dump(a), string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));

  size_t offset = 0;
  EXPECT_EQ(c.Load(c.Dump(a), &offset), 0xdeadbeef);
  EXPECT_EQ(offset, sizeof(uint32_t));
}

TEST(Converter, vector) {
  protocol::converter::Converter<vector<int>> c;

  vector<int> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);

  size_t offset = 0;
  vector<int> b = c.Load(c.Dump(a), &offset);
  ASSERT_EQ(a.size(), b.size());
  for (size_t i = 0; i < a.size(); ++i) {
    EXPECT_EQ(a[i], b[i]);
  }
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
    EXPECT_EQ(protocol::converter::Dump(a),
              string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));
  }

  {
    char ans_buf[] = {
      '\xef', '\xbe', '\xad', '\xde',
      '\x04', '\x00', '\x00', '\x00',
      '\x00', '\x00', '\x00', '\x00',
      'm', 'e', 'o', 'w'
    };
    EXPECT_EQ(protocol::converter::Dump(a, s),
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
    EXPECT_EQ(protocol::converter::Dump(a, s, v),
              string(ans_buf, sizeof(ans_buf) / sizeof(ans_buf[0])));
  }
}


TEST(Load, All) {

  {
    uint32_t a;
    char buf[] = {'\xef', '\xbe', '\xad', '\xde'};
    size_t size = sizeof(buf) / sizeof(buf[0]);
    size_t offset = 0;
    protocol::converter::LoadTo(string(buf, size), &offset, &a);
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
    protocol::converter::LoadTo(string(buf, size), &offset, &a, &s);
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
    protocol::converter::LoadTo(string(buf, size), &offset, &a, &s, &v);
    EXPECT_EQ(a, 0xdeadbeef);
    EXPECT_EQ(s, "meow");
    EXPECT_EQ(string(&v[0], v.size()), "abc");
  }
}

}  // namespace
