#ifndef ARTICLE_ANALYSIS_SERVER_SERVER_H_
#define ARTICLE_ANALYSIS_SERVER_SERVER_H_


#include <stdint.h>
#include <string>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "analyst/i_analyst.h"
#include "logging/logger.h"
#include "utils/options.h"


namespace server {


class Options: public utils::AOptionCollection {
 public:
  Options() : AOptionCollection("Server options.") {
    AddOption<utils::TypedOption<std::string>>(
        "bind_address", "127.0.0.1", "Bind address");
    AddOption<utils::TypedOption<uint16_t>>(
        "bind_port", 8997, "Bind port");
  }
};


/**
 * A simple server for handle the outcome queries.
 */
class Server {
 public:
  Server(Options const& opts,
         analyst::IAnalyst const* analyst,
         logging::Logger const* parent_log,
         bool debug=false);

  ~Server();

  /**
   * Start running the main loop.
   */
  void RunMainLoop();

 private:
  void InitSocket_();

  bool RecvAll_(size_t sz, void* buf);

  bool RecvAll_(size_t sz, std::string* ret);
  
  bool SendAll_(std::string const& s);

  bool is_debug_;
  std::string bind_address_;
  uint16_t bind_port_;
  int sock_fd_;
  int conn_fd_;
  analyst::IAnalyst const* analyst_;
  logging::Logger* logger_;
};

}  // namespace server

#endif  // ARTICLE_ANALYSIS_SERVER_SERVER_H_
