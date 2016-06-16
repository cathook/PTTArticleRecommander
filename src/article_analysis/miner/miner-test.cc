#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <array>
#include <memory>
#include <string>
#include <thread>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "logging/logger.h"
#include "miner/miner.h"
#include "protocol/net.h"
#include "protocol/types.h"


using std::array;
using std::string;
using std::thread;
using std::unique_ptr;
using std::vector;
using protocol::net::Dump;
using protocol::net::Load;
using protocol::net::PackageHeader;
using protocol::net::PackageType;
using protocol::types::Board;
using protocol::types::DocMetaData;
using protocol::types::DocRealData;
using protocol::types::Identity;
using protocol::types::Time;
using protocol::types::User;


namespace {


class FakeServer {
 public:
  static uint16_t port;

  FakeServer() { 
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock_fd_ >= 0);

    for (port = 8993; ; ++port) {
      struct sockaddr_in addr;
      memset(reinterpret_cast<void*>(&addr), 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

      int ret = bind(
          sock_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
      if (ret == 0) {
        break;
      }
    }

    listen(sock_fd_, 5);
  }

  ~FakeServer() {
    close(sock_fd_);
    close(new_sock_fd_);
  }

  bool MainLoop() {
    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    new_sock_fd_ = accept(
        sock_fd_, reinterpret_cast<struct sockaddr*>(&cli_addr), &cli_len);
    if (new_sock_fd_ < 0) {
      printf("bye, %s\n", strerror(errno));
      exit(1);
    }

    PackageHeader ph;
    while (true) {
      string s;
      if (!RecvAll_(sizeof(ph), &s)) {
        return false;
      }
      size_t offs = 0;
      if (!Load(s, &offs, &ph)) {
        return false;
      }
      if (offs != s.length()) {
        return false;
      }
      if (ph.size == 12345) {
        return true;
      }
      int ret;
      if ((ret = HandleGetMaxId_(ph)) == 0 &&
          (ret = HandleGetDocMetaDataAfterId_(ph)) == 0 &&
          (ret = HandleGetDocMetaDataAfterTime_(ph)) == 0 &&
          (ret = HandleGetDocMetaDataOfAuthor_(ph)) == 0 &&
          (ret = HandleGetDocMetaDataSeries_(ph)) == 0 &&
          (ret = HandleGetDocRealData_(ph)) == 0) {
        return false;
      }
      if (ret == 1) {
        return false;
      }
    }
  }

 private:
  int HandleGetMaxId_(PackageHeader const& ph) {
    if (ph.type != PackageType::QUERY_MAX_ID) return 0;

    Board b;
    if (!RecvObj_(ph.size, &b)) return 1;

    Identity id = atoi(b.c_str());
    if (!SendObj_(PackageType::REPLY_MAX_ID, id)) return 1;

    return 2;
  }

  int HandleGetDocMetaDataAfterId_(PackageHeader const& ph) {
    if (ph.type != PackageType::QUERY_DOC_META_DATA_AFTER_ID) return 0;
    return HandleGetDocMetaData_<Identity>(ph, 1);
  }

  int HandleGetDocMetaDataAfterTime_(PackageHeader const& ph) {
    if (ph.type != PackageType::QUERY_DOC_META_DATA_AFTER_TIME) return 0;
    return HandleGetDocMetaData_<Time>(ph, 2);
  }

  int HandleGetDocMetaDataOfAuthor_(PackageHeader const& ph) {
    if (ph.type != PackageType::QUERY_DOC_META_DATA_OF_AUTHOR) return 0;
    return HandleGetDocMetaData_<User>(ph, 3);
  }

  int HandleGetDocMetaDataSeries_(PackageHeader const& ph) {
    if (ph.type != PackageType::QUERY_DOC_META_DATA_SERIES) return 0;
    return HandleGetDocMetaData_<Identity>(ph, 4);
  }

  int HandleGetDocRealData_(PackageHeader const& ph) {
    if (ph.type != PackageType::QUERY_DOC_REAL_DATA) return 0;

    Board b;
    Identity i;
    if (!RecvObj_(ph.size, &b, &i)) return 1;

    DocRealData drd;
    drd.content = L"meow";

    if (!SendObj_(PackageType::REPLY_DOC_REAL_DATA, drd)) return 1;

    return 2;
  }

  template <typename T>
  int HandleGetDocMetaData_(PackageHeader const& ph, size_t n) {
    Board b;
    T t;
    if (!RecvObj_(ph.size, &b, &t)) return 1;
    vector<DocMetaData> a;
    array<uint32_t, 3> xx;
    xx[0] = 1;
    xx[1] = 2;
    xx[2] = 3;
    for (size_t nn = 0; nn < n; ++nn) {
      a.emplace_back(3, 3, L"meow", "aaa", 0, b, xx);
    }
    uint32_t t1 = static_cast<uint32_t>(ph.type);
    uint32_t bit = static_cast<uint32_t>(PackageType::REPLY_QUERY_BIT);
    if (!SendObj_(static_cast<PackageType>(t1 ^ bit), a)) return 1;
    return 2;
  }

  template <typename T>
  bool SendObj_(PackageType const& type, T const& obj) {
    string s = Dump(obj);
    PackageHeader ph(type, s.length());
    return SendAll_(sizeof(ph) + s.length(), Dump(ph) + s);
  }

  template <typename Type>
  bool RecvObj_(size_t sz, Type* ret) {
    string s;
    if (!RecvAll_(sz, &s)) {
      return false;
    }
    size_t offs = 0;
    if (!protocol::net::Load(s, &offs, ret)) {
      return false;
    }
    return (offs == sz);
  }

  template <typename Type, typename Type2>
  bool RecvObj_(size_t sz, Type* ret, Type2* ret2) {
    string s;
    if (!RecvAll_(sz, &s)) {
      return false;
    }
    size_t offs = 0;
    if (!protocol::net::Load(s, &offs, ret) ||
        !protocol::net::Load(s, &offs, ret2)) {
      return false;
    }
    return (offs == sz);
  }

  bool RecvAll_(size_t sz, string* s) {
    unique_ptr<char[]> buf(new char[sz]);
    size_t recved = 0;
    while (recved < sz) {
      ssize_t a = recv(new_sock_fd_, buf.get() + recved, sz - recved, 0);
      if (a <= 0) {
        return false;
      }
      recved += a;
    }
    *s = string(buf.get(), sz);
    return true;
  }

  bool SendAll_(size_t sz, string const& s) {
    size_t sent = 0;
    while (sent < s.length()) {
      ssize_t a = send(new_sock_fd_, s.data() + sent, s.length() - sent, 0);
      if (a <= 0) {
        return false;
      }
      sent += a;
    }
    return true;
  }

  int sock_fd_;
  int new_sock_fd_;
};


uint16_t FakeServer::port = 0;


void RunFakeServer(bool* is_bad_flag) {
  FakeServer fs;
  *is_bad_flag = !fs.MainLoop();
}


class MinerTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    is_bad = false;
    thr.reset(new thread(RunFakeServer, &is_bad));
    sleep(3);
    miner::Options opt;
    opt.GetOption<utils::TypedOption<uint16_t>>("server_port")->set_value(
        FakeServer::port);
    miner = new miner::Miner(opt, logging::GetRootLogger());
  }

  void TearDown() override {
    delete miner;
    thr->join();
  }

  bool is_bad;
  unique_ptr<thread> thr;
  miner::Miner* miner;
};


TEST_F(MinerTestFixture, All) {
  {
    auto id = miner->GetMaxId("123");
    EXPECT_EQ(id, 123);
    printf("done1\n");
  }

  {
    auto v = miner->GetDocMetaDataAfterId("board", 123);
    ASSERT_EQ(v.size(), 1);
    for (auto& k : v) {
      EXPECT_EQ(k.id, 3);
      EXPECT_EQ(k.prev_id, 3);
      EXPECT_EQ(k.title, L"meow");
      EXPECT_EQ(k.author, "aaa");
      EXPECT_EQ(k.post_time, 0);
      EXPECT_EQ(k.board, "board");
      EXPECT_EQ(k.num_reply_rows[0], 1);
      EXPECT_EQ(k.num_reply_rows[1], 2);
      EXPECT_EQ(k.num_reply_rows[2], 3);
    }
    printf("done2\n");
  }

  {
    auto v = miner->GetDocMetaDataAfterTime("board", 123);
    ASSERT_EQ(v.size(), 2);
    for (auto& k : v) {
      EXPECT_EQ(k.id, 3);
      EXPECT_EQ(k.prev_id, 3);
      EXPECT_EQ(k.title, L"meow");
      EXPECT_EQ(k.author, "aaa");
      EXPECT_EQ(k.post_time, 0);
      EXPECT_EQ(k.board, "board");
      EXPECT_EQ(k.num_reply_rows[0], 1);
      EXPECT_EQ(k.num_reply_rows[1], 2);
      EXPECT_EQ(k.num_reply_rows[2], 3);
    }
    printf("done3\n");
  }

  {
    auto v = miner->GetDocMetaDataOfAuthor("bomm", "author");
    ASSERT_EQ(v.size(), 3);
    for (auto& k : v) {
      EXPECT_EQ(k.id, 3);
      EXPECT_EQ(k.prev_id, 3);
      EXPECT_EQ(k.title, L"meow");
      EXPECT_EQ(k.author, "aaa");
      EXPECT_EQ(k.post_time, 0);
      EXPECT_EQ(k.board, "bomm");
      EXPECT_EQ(k.num_reply_rows[0], 1);
      EXPECT_EQ(k.num_reply_rows[1], 2);
      EXPECT_EQ(k.num_reply_rows[2], 3);
    }
    printf("done4\n");
  }

  {
    auto v = miner->GetDocMetaDataSeries("wwww", 13);
    ASSERT_EQ(v.size(), 4);
    for (auto& k : v) {
      EXPECT_EQ(k.id, 3);
      EXPECT_EQ(k.prev_id, 3);
      EXPECT_EQ(k.title, L"meow");
      EXPECT_EQ(k.author, "aaa");
      EXPECT_EQ(k.post_time, 0);
      EXPECT_EQ(k.board, "wwww");
      EXPECT_EQ(k.num_reply_rows[0], 1);
      EXPECT_EQ(k.num_reply_rows[1], 2);
      EXPECT_EQ(k.num_reply_rows[2], 3);
    }
    printf("done5\n");
  }

  {
    auto d = miner->GetDocRealData("haha", 514);
    EXPECT_EQ(d.content, L"meow");
    EXPECT_EQ(d.reply_messages.size(), 0);
    printf("done6\n");
  }
}

}  // namespace
