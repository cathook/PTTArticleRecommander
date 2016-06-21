#ifndef ARTICLE_ANALYSIS_PROTOCOL_TYPES_H_
#define ARTICLE_ANALYSIS_PROTOCOL_TYPES_H_


#include <stdint.h>
#include <string.h>

#include <array>
#include <string>
#include <vector>

#include "protocol/i_loadable.h"
#include "protocol/i_dumpable.h"
#include "protocol/net.h"


#pragma pack(push, 1)


namespace protocol {

namespace types {


/**
 * Type of the document's title string.
 */
typedef std::wstring DocTitle;


/**
 * Type of the user name string.
 */
typedef std::string User;


/**
 * Type of the document's content string.
 */
typedef std::string Content;


/**
 * Type of of the time point.
 */
typedef int64_t Time;


/**
 * Type of of the id of a document.
 */
typedef int32_t Identity;


/**
 * Type of the board.
 */
typedef std::string Board;


/**
 * Enumerate of the reply mode
 */
enum class ReplyMode : uint16_t {
  GOOD = 0,
  NORMAL = 1,
  WOO = 2,
};


/**
 * Number of different reply modes.
 */
static constexpr size_t NUM_REPLY_MODES = 3;


/**
 * Fully identity of a document.
 */
struct DocIdentity : IDumpable, ILoadable {
  Board board;
  Identity id;

  DocIdentity() {}

  DocIdentity(Board const& board, Identity const& id) : board(board), id(id) {}

  std::string Dump() const override final { return net::Dump(board, id); }

  bool Load(std::string const& s, size_t* offs) override final {
    return net::Load(s, offs, &board, &id);
  }
};


/**
 * Information results from the analyst.
 */
struct DocRelInfo : IDumpable, ILoadable {
  /**
   * The positive relative document's identity.
   */
  std::vector<DocIdentity> pos_rel_docs;

  /**
   * The negative relative document's identity.
   */
  std::vector<DocIdentity> neg_rel_docs;

  /**
   * The neutral relative document's identity.
   */
  std::vector<DocIdentity> neutral_rel_docs;

  DocRelInfo() {}

  DocRelInfo(std::vector<DocIdentity> const& pos_rel_docs,
             std::vector<DocIdentity> const& neg_rel_docs,
             std::vector<DocIdentity> const& neutral_rel_docs) :
      pos_rel_docs(pos_rel_docs),
      neg_rel_docs(neg_rel_docs),
      neutral_rel_docs(neutral_rel_docs) {}

  std::string Dump() const override final {
    return net::Dump(pos_rel_docs, neg_rel_docs, neutral_rel_docs);
  }

  bool Load(std::string const& buf, size_t* offs) override final {
    return net::Load(buf, offs,
                     &pos_rel_docs, &neg_rel_docs, &neutral_rel_docs);
  }
};


/**
 * Type of a row of reply message.
 */
struct ReplyMessage : ILoadable {
  ReplyMode mode;
  User user;
  Content message;

  ReplyMessage() {}

  ReplyMessage(ReplyMode mode, User const& user, Content const& message) :
      mode(mode), user(user), message(message) {}

  bool Load(std::string const& s, size_t* offset) override final {
    return net::Load(s, offset, &mode, &user, &message);
  }
};


/**
 * Type of an array of reply messages.
 */
typedef std::vector<ReplyMessage> ReplyMessages;


/**
 * Meta data of a document.
 */
struct DocMetaData : IDumpable, ILoadable {
  Identity id;
  Identity prev_id;
  DocTitle title;
  User author;
  Time post_time;
  Board board;
  std::array<uint32_t, NUM_REPLY_MODES> num_reply_rows;

  DocMetaData() {}

  DocMetaData(Identity id,
              Identity prev_id,
              DocTitle const& title,
              User const& author,
              Time const& post_time,
              Board const& board,
              std::array<uint32_t, NUM_REPLY_MODES> const& num_reply_rows) :
      id(id), prev_id(prev_id),
      title(title), author(author), post_time(post_time), board(board),
      num_reply_rows(num_reply_rows) {}

  std::string Dump() const override final {
    return net::Dump(
        id, prev_id, title, author, post_time, board, num_reply_rows);
  }

  bool Load(std::string const& buf, size_t* offset) override final {
    return net::Load(buf, offset,
                     &id, &prev_id,
                     &title, &author, &post_time, &board,
                     &num_reply_rows);
  }
};


/**
 * Real document content data.
 */
struct DocRealData : IDumpable, ILoadable {
  Content content;
  ReplyMessages reply_messages;

  DocRealData() {}

  DocRealData(Content const& content, ReplyMessages const& reply_messages) :
      content(content), reply_messages(reply_messages) {}

  std::string Dump() const override final {
    return net::Dump(content, reply_messages);
  }

  bool Load(std::string const& buf, size_t* offset) override final {
    return net::Load(buf, offset, &content, &reply_messages);
  }
};

}  // namespace types

}  // namespace protocol


#pragma pack(pop)

#endif  // ARTICLE_ANALYSIS_PROTOCOL_TYPES_H_
