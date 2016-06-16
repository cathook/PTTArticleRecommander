#include "server/server.h"

#include <string.h>

#include <memory>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol/net.h"


using std::string;
using std::unique_ptr;


namespace server {


Server::Server(Options const& opts,
               analyst::IAnalyst const* analyst,
               logging::Logger const* parent_log,
               bool debug) {
  bind_address_ =
      opts.GetOption<utils::TypedOption<std::string>>("bind_address")->value();
  bind_port_ =
      opts.GetOption<utils::TypedOption<uint16_t>>("bind_port")->value();
  analyst_ = analyst;
  logger_ = parent_log->CreateSubLogger("Server");
  is_debug_ = debug;

  InitSocket_();
}

Server::~Server() {
  close(sock_fd_);
  delete logger_;
}

void Server::RunMainLoop() {
  // TODO(cathook): Refactor this fat function.
  while (true) {
    logger_->Info("Wait for income connection.");

    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    conn_fd_ = accept(
        sock_fd_, reinterpret_cast<struct sockaddr*>(&cli_addr), &cli_len);
    if (conn_fd_ < 0) {
      logger_->Warn("Accept error: %s, Ignore.", strerror(errno));
      continue;
    }
    while (true) {
      protocol::net::PackageHeader header;
      if (!RecvAll_(sizeof(header), reinterpret_cast<void*>(&header))) {
        logger_->Warn("Cannot get full package header, close.");
        break;
      }

      if (is_debug_ &&
          header.type == protocol::net::PackageType::DEBUG_CLOSE_NOTIFY) {
        return;
      }

      logger_->Info("Got an query.");

      if (header.type != protocol::net::PackageType::QUERY_DOC_REL_INFO) {
        logger_->Warn("Unknown package type, close.");
        break;
      }

      string s;
      if (!RecvAll_(header.size, &s)) {
        logger_->Warn("Cannot recv package content, close.");
        break;
      }

      protocol::types::DocIdentity doc_id;
      size_t offs = 0;
      if (!protocol::net::Load(s, &offs, &doc_id) || offs != s.size()) {
        logger_->Warn("Wrong format.");
        break;
      }

      auto ret_buf = protocol::net::Dump(analyst_->GetDocInfo(doc_id));
      protocol::net::PackageHeader ph(
          protocol::net::PackageType::REPLY_DOC_REL_INFO, ret_buf.length());
      if (!SendAll_(protocol::net::Dump(ph) + ret_buf)) {
        logger_->Warn("Cannot send.");
        break;
      }

      logger_->Info("Handled.");
    }

    close(conn_fd_);
  }
}

void Server::InitSocket_() {
  sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd_ < 0) {
    logger_->Fatal("Cannot create socket.");
  }

  struct sockaddr_in addr;
  memset(reinterpret_cast<void*>(&addr), 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(bind_port_);
  addr.sin_addr.s_addr = inet_addr(bind_address_.c_str());
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int ret = bind(
      sock_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  if (ret != 0) {
    logger_->Fatal("Cannot bind address (%s, %d)\n",
                   bind_address_, static_cast<int>(bind_port_));
  }
  listen(sock_fd_, 5);

  logger_->Info("Server started.");
}

bool Server::RecvAll_(size_t sz, void* buf) {
  size_t got = 0;
  while (got < sz) {
    ssize_t a = recv(conn_fd_, (char*)buf + got, sz - got, 0);
    if (a <= 0) {
      return false;
    }
    got += a;
  }
  return true;
}

bool Server::RecvAll_(size_t sz, string* ret) {
  unique_ptr<char[]> buf(new char[sz]);
  if (!RecvAll_(sz, reinterpret_cast<void*>(buf.get()))) {
    return false;
  }
  *ret = string(buf.get(), sz);
  return true;
}

bool Server::SendAll_(string const& s) {
  size_t got = 0;
  while (got < s.length()) {
    ssize_t a = send(conn_fd_, s.data() + got, s.length() - got, 0);
    if (a <= 0) {
      return false;
    }
    got += a;
  }
  return true;
}

}  // namespace server
