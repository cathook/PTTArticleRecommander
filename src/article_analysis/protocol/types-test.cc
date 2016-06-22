#include <string>

#include <gtest/gtest.h>

#include "protocol/types.h"


using std::string;


namespace {


TEST(DocIdentity, All) {
  string s("\x01\x00\x00\x00\x00\x00\x00\x00""a"
           "\x03\x00\x00\x00", 8 + 1 + 4);
  protocol::types::DocIdentity di;
  size_t offs = 0;
  bool ret = di.Load(s, &offs);
  EXPECT_TRUE(ret);
  EXPECT_EQ(offs, 8u + 1 + 4);
  EXPECT_EQ(di.board, "a");
  EXPECT_EQ(di.id, 3);
}


TEST(ReplyMessage, Load) {
  protocol::types::ReplyMessage rm;

  string s("\x01\x00"
           "\x03\x00\x00\x00\x00\x00\x00\x00""abc"
           "\x02\x00\x00\x00\x00\x00\x00\x00""ab",
           2 + 8 + 3 + 8 + 2);
  size_t offs = 0;
  bool ret = rm.Load(s, &offs);
  EXPECT_TRUE(ret);
  EXPECT_EQ(offs, s.length());
  EXPECT_EQ(rm.mode, protocol::types::ReplyMode::NORMAL);
  EXPECT_EQ(rm.user, "abc");
  EXPECT_EQ(rm.message, "ab");
}


TEST(DocMetaData, Load) {
  protocol::types::DocMetaData dm;
  string s("\x02\x00\x00\x00"
           "\x01\x00\x00\x00"
           "\x08\x00\x00\x00\x00\x00\x00\x00""abcdmeow"
           "\x03\x00\x00\x00\x00\x00\x00\x00""xyz"
           "\x04\x00\x00\x00\x00\x00\x00\x00"
           "\x05\x00\x00\x00\x00\x00\x00\x00""12345"
           "\x0a\x00\x00\x00"
           "\x0b\x00\x00\x00"
           "\x0c\x00\x00\x00",
           4 + 4 + (8 + 8) + (8 + 3) + 8 + (8 + 5) + 4 + 4 + 4);
  size_t offs = 0;
  bool ret = dm.Load(s, &offs);
  EXPECT_TRUE(ret);
  EXPECT_EQ(offs, s.length());
  EXPECT_EQ(dm.id, 2);
  EXPECT_EQ(dm.prev_id, 1);
  EXPECT_EQ(dm.title, "abcdmeow");
  EXPECT_EQ(dm.author, "xyz");
  EXPECT_EQ(dm.post_time, 4);
  EXPECT_EQ(dm.board, "12345");
  EXPECT_EQ(dm.num_reply_rows[0], 10u);
  EXPECT_EQ(dm.num_reply_rows[1], 11u);
  EXPECT_EQ(dm.num_reply_rows[2], 12u);
}


TEST(DocRealData, Load) {
  protocol::types::DocRealData dd;
  string s("\x07\x00\x00\x00\x00\x00\x00\x00""1234567"
           "\x01\x00\x00\x00\x00\x00\x00\x00"
           "\x01\x00"
           "\x03\x00\x00\x00\x00\x00\x00\x00""abc"
           "\x02\x00\x00\x00\x00\x00\x00\x00""ab",
           (8 + 7) + 8 + 2 + (8 + 3) + (8 + 2));
  size_t offs = 0;
  bool ret = dd.Load(s, &offs);
  EXPECT_TRUE(ret);
  EXPECT_EQ(offs, s.length());
  EXPECT_EQ(dd.content, "1234567");
  ASSERT_EQ(dd.reply_messages.size(), 1u);
  EXPECT_EQ(dd.reply_messages[0].mode, protocol::types::ReplyMode::NORMAL);
  EXPECT_EQ(dd.reply_messages[0].user, "abc");
  EXPECT_EQ(dd.reply_messages[0].message, "ab");
}

}  // namespace
