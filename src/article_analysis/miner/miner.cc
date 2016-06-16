#include "miner/miner.h"

#include <string.h>

#include <vector>
#include <memory>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol/types.h"
#include "protocol/net.h"


using std::string;
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


namespace miner {

namespace {


string DumpPackage_(PackageType const& type, string const& buf) {
  PackageHeader header(type, buf.size());
  return Dump(header) + buf;
}

}  // namespace


Miner::Miner(Options const& options, logging::Logger* parent_logger) {
  server_addr_ =
      options.GetOption<utils::TypedOption<string>>("server_address")->value();
  server_port_ =
      options.GetOption<utils::TypedOption<uint16_t>>("server_port")->value();

  logger_ = parent_logger->CreateSubLogger("Miner");

  InitSocket_();
}

Miner::~Miner() {
  close(sock_fd_);
  delete logger_;
}

Identity Miner::GetMaxId(Board const& board) {
  SendAll_(DumpPackage_(PackageType::QUERY_MAX_ID, Dump(board)));
  PackageHeader header(LoadHeader_());
  if (header.type != PackageType::REPLY_MAX_ID) {
    logger_->Fatal("Wrong reply type!");
  }
  Identity ret(0);
  size_t offs = 0;
  Load(RecvAll_(header.size), &offs, &ret);
  if (offs != header.size) {
    logger_->Fatal("Wrong format!");
  }
  return ret;
}

vector<DocMetaData> Miner::GetDocMetaDataAfterId(Board const& board,
                                                 Identity const& id) {
  return GetDocMetaDataCommon_(PackageType::QUERY_DOC_META_DATA_AFTER_ID,
                               Dump(board, id));
}

vector<DocMetaData> Miner::GetDocMetaDataAfterTime(Board const& board,
                                                   Time const& post_time) {
  return GetDocMetaDataCommon_(PackageType::QUERY_DOC_META_DATA_AFTER_TIME,
                               Dump(board, post_time));
}

vector<DocMetaData> Miner::GetDocMetaDataOfAuthor(Board const& board,
                                                  User const& author) {
  return GetDocMetaDataCommon_(PackageType::QUERY_DOC_META_DATA_OF_AUTHOR,
                               Dump(board, author));
}

vector<DocMetaData> Miner::GetDocMetaDataSeries(Board const& board,
                                                Identity const& id) {
  return GetDocMetaDataCommon_(PackageType::QUERY_DOC_META_DATA_SERIES,
                               Dump(board, id));
}

DocRealData Miner::GetDocRealData(Board const& board, Identity const& id) {
  SendAll_(DumpPackage_(PackageType::QUERY_DOC_REAL_DATA, Dump(board, id)));
  PackageHeader header(LoadHeader_());
  if (header.type != PackageType::REPLY_DOC_REAL_DATA) {
    logger_->Fatal("Wrong reply type!");
  }
  DocRealData ret;
  size_t offs = 0;
  Load(RecvAll_(header.size), &offs, &ret);
  if (offs != header.size) {
    logger_->Fatal("Wrong format!");
  }
  return ret;
}

void Miner::InitSocket_() {
  sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd_ < 0) {
    logger_->Fatal("Cannot create the socket: %s.\n", strerror(errno));
  }

  struct sockaddr_in addr;
  memset(reinterpret_cast<void*>(&addr), 0, sizeof(addr));
  struct hostent* host = gethostbyname(server_addr_.c_str());
  if (host == NULL) {
    logger_->Fatal("Cannot get the host info: %s.", strerror(errno));
  }
  addr.sin_family = AF_INET;
  memcpy(reinterpret_cast<void*>(&addr.sin_addr.s_addr),
         host->h_addr, host->h_length);
  addr.sin_port = htons(server_port_);

  if (connect(sock_fd_,
              reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
    logger_->Fatal("Cannot connect to the server: %s.", strerror(errno));
  }

  logger_->Info("Connected to the server: %s:%d.", server_addr_, server_port_);
}

void Miner::SendAll_(string const& s) {
  size_t sent = 0;
  while (sent < s.length()) {
    ssize_t ret = send(sock_fd_, s.data() + sent, s.length() - sent, 0);
    if (ret <= 0) {
      logger_->Fatal("Cannot send message: %s.\n", strerror(errno));
    }
    sent += ret;
  }
}

string Miner::RecvAll_(size_t size) {
  unique_ptr<char[]> buf(new char[size]);
  size_t recved = 0;
  while (recved < size) {
    ssize_t ret = recv(sock_fd_, buf.get() + recved, size - recved, 0);
    if (ret <= 0) {
      logger_->Fatal("Cannot recv message: %s.\n", strerror(errno));
    }
    recved += ret;
  }
  return string(buf.get(), size);
}

PackageHeader Miner::LoadHeader_() {
  size_t offs = 0;
  PackageHeader ret;
  Load(RecvAll_(sizeof(ret)), &offs, &ret);
  return ret;
}

vector<DocMetaData> Miner::GetDocMetaDataCommon_(PackageType type,
                                                 string const& buf) {
  SendAll_(DumpPackage_(type, buf));
  PackageHeader header(LoadHeader_());
  uint32_t t1 = static_cast<uint32_t>(type);
  uint32_t t2 = static_cast<uint32_t>(header.type);
  if (t1 != (t2 ^ static_cast<uint32_t>(PackageType::REPLY_QUERY_BIT))) {
    logger_->Fatal("Wrong reply type!");
  }
  vector<DocMetaData> ret;
  size_t offs = 0;
  if (!Load(RecvAll_(header.size), &offs, &ret)) {
    logger_->Fatal("Wrong format!");
  }
  if (offs != header.size) {
    logger_->Fatal("Wrong format!");
  }
  return ret;
}

}  // namespace miner
