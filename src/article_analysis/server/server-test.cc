#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "analyst/i_analyst.h"
#include "logging/logger.h"
#include "protocol/net.h"
#include "server/server.h"


using std::string;
using std::thread;
using std::unique_ptr;
using std::vector;


namespace {


class FakeAnalyst : public analyst::IAnalyst {
 public:
  analyst::DocRelInfo GetDocRelInfo(
      analyst::DocIdentity const& doc_id) const override final {
    analyst::DocRelInfo ret(vector<analyst::DocIdentity>(2, doc_id),
                            vector<analyst::DocIdentity>(1, doc_id),
                            vector<analyst::DocIdentity>(3, doc_id));
    return ret;
  }
};


class ServerTestFeature : public ::testing::Test {
 protected:
  void SetUp() override {
    ia = new FakeAnalyst();

    s = new server::Server(
        server::Options(), ia, logging::GetRootLogger(), true);

    thr.reset(new thread([this]() { s->RunMainLoop(); }));
    sleep(1);

    InitConnFd_();
  }

  void TearDown() override {
    protocol::net::PackageHeader header(
        protocol::net::PackageType::DEBUG_CLOSE_NOTIFY, 0);
    if (!SendAll(protocol::net::Dump(header))) {
      assert(false);
    }
    thr->join();
    delete s;
    delete ia;
    close(conn_fd);
  }

  bool SendAll(string const& s) {
    size_t sent = 0;
    while (sent < s.length()) {
      ssize_t a = send(conn_fd, s.data() + sent, s.length() - sent, 0);
      if (a <= 0) {
        return false;
      }
      sent += a;
    }
    return true;
  }

  bool RecvAll(size_t sz, string* ret) {
    size_t got = 0;
    unique_ptr<char[]> ptr(new char[sz]);
    while (got < sz) {
      ssize_t a = recv(conn_fd, ptr.get() + got, sz - got, 0);
      if (a <= 0) {
        return false;
      }
      got += a;
    }
    *ret = string(ptr.get(), sz);
    return true;
  }

  int conn_fd;
  analyst::IAnalyst* ia;
  server::Server* s;
  unique_ptr<thread> thr;

 private:
  void InitConnFd_() {
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(conn_fd >= 0);

    struct sockaddr_in addr;
    memset(reinterpret_cast<void*>(&addr), 0, sizeof(addr));
    struct hostent* host = gethostbyname("localhost");
    assert(host != NULL);
    addr.sin_family = AF_INET;
    memcpy(reinterpret_cast<void*>(&addr.sin_addr.s_addr),
           host->h_addr, host->h_length);
    addr.sin_port = htons(8997);

    int ret = connect(
        conn_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    assert(ret == 0);
  }
};


TEST_F(ServerTestFeature, All) {
  for (int i = 0; i < 10; ++i) {
    protocol::types::DocIdentity di("meow", 123);
    protocol::net::PackageHeader header(
        protocol::net::PackageType::QUERY_DOC_REL_INFO,
        di.Dump().length());
    SendAll(protocol::net::Dump(header) + di.Dump());
    string s;
    bool ret = RecvAll(sizeof(header), &s);
    EXPECT_TRUE(ret);
    size_t offs = 0;
    protocol::net::Load(s, &offs, &header);
    EXPECT_EQ(header.type, protocol::net::PackageType::REPLY_DOC_REL_INFO);
    ret = RecvAll(header.size, &s);
    EXPECT_TRUE(ret);
    protocol::types::DocRelInfo dri;
    offs = 0;
    ret = dri.Load(s, &offs);
    EXPECT_TRUE(ret);
    EXPECT_EQ(offs, s.length());
    ASSERT_EQ(dri.pos_rel_docs.size(), 2);
    ASSERT_EQ(dri.neg_rel_docs.size(), 1);
    ASSERT_EQ(dri.neutral_rel_docs.size(), 3);
    for (auto& a : dri.pos_rel_docs) {
      EXPECT_EQ(a.board, di.board);
      EXPECT_EQ(a.id, di.id);
    }
    for (auto& a : dri.neg_rel_docs) {
      EXPECT_EQ(a.board, di.board);
      EXPECT_EQ(a.id, di.id);
    }
    for (auto& a : dri.neutral_rel_docs) {
      EXPECT_EQ(a.board, di.board);
      EXPECT_EQ(a.id, di.id);
    }
  }
}

}  // namespace
