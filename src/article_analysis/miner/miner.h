#ifndef ARTICLE_ANALYSIS_MINER_MINER_H_
#define ARTICLE_ANALYSIS_MINER_MINER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "protocol/types.h"
#include "utils/options.h"
#include "logging/logger.h"


namespace miner {


/**
 * Required options of a miner
 */
class Options : public utils::AOptionCollection {
 public:
  Options() : AOptionCollection("Options about the miner.") {
    AddOption<utils::TypedOption<std::string>>(
        "server_address", "localhost",
        "The server address of the real backend server.");
    AddOption<utils::TypedOption<uint16_t>>(
        "server_port", 8993, "The server port.");
  }
};


/**
 * A miner.
 */
class Miner {
 public:
  Miner(Options const& options, logging::Logger* parent_logger);

  ~Miner();

  /**
   * Gets the maximum document id in the specific board.
   *
   * @param [in] board The specific board name.
   * @return The maxiumu document id.
   */
  protocol::types::Identity GetMaxId(protocol::types::Board const& board);

  /**
   * Gets the document meta data which is larger than or equal to some id.
   *
   * @param [in] board The specific board name.
   * @param [in] id The smallest acceptable document id.
   * @return An array of document meta data.
   */
  std::vector<protocol::types::DocMetaData> GetDocMetaDataAfterId(
      protocol::types::Board const& board,
      protocol::types::Identity const& id);

  /**
   * Gets the document meta data which is posted later that some time point.
   *
   * @param [in] board The specific board name.
   * @param [in] post_time The time point.
   * @return An array of document meta data.
   */
  std::vector<protocol::types::DocMetaData> GetDocMetaDataAfterTime(
      protocol::types::Board const& board,
      protocol::types::Time const& post_time);

  /**
   * Gets all the meta data of the documents from the specific author.
   *
   * @param [in] board The specific board name.
   * @param [in] author The author.
   * @return An array of document meta data.
   */
  std::vector<protocol::types::DocMetaData> GetDocMetaDataOfAuthor(
      protocol::types::Board const& board,
      protocol::types::User const& author);

  /**
   * Gets a series of document which has the same theme.
   *
   * @param [in] board The specific board name.
   * @param [in] id One of the document's id.
   * @return An array of document meta data.
   */
  std::vector<protocol::types::DocMetaData> GetDocMetaDataSeries(
      protocol::types::Board const& board,
      protocol::types::Identity const& id);

  /**
   * Gets a document's real data by its id.
   *
   * @param [in] board The specific board name.
   * @param [in] id One of the document's id.
   * @return The document's real id.
   */
  protocol::types::DocRealData GetDocRealData(
      protocol::types::Board const& board,
      protocol::types::Identity const& id);

  int sock_fd() const { return sock_fd_; }

  Miner(Miner const& rhs) = delete;

  Miner& operator=(Miner const& rhs) = delete;

 private:
  void InitSocket_();

  void SendAll_(std::string const& s);

  std::string RecvAll_(size_t size);

  protocol::net::PackageHeader LoadHeader_();

  std::vector<protocol::types::DocMetaData> GetDocMetaDataCommon_(
      protocol::net::PackageType type, std::string const& buf);

  std::string server_addr_;
  uint16_t server_port_;
  int sock_fd_;

  logging::Logger* logger;
};

}  // namespace miner

#endif  // ARTICLE_ANALYSIS_MINER_MINER_H_
