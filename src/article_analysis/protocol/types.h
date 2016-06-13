#ifndef ARTICLE_ANALYSIS_PROTOCOL_TYPES_H_
#define ARTICLE_ANALYSIS_PROTOCOL_TYPES_H_


#include <string.h>
#include <time.h>

#include <string>
#include <vector>


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
typedef std::wstring Content;


/**
 * Type of of the time point.
 */
typedef time_t Time;


/**
 * Type of the board.
 */
typedef std::string Board;


/**
 * Enumerate of the reply mode
 */
enum class ReplyMode : int {
  GOOD = 0,
  NORMAL,
  WOO,
};


/**
 * Number of different reply modes.
 */
static constexpr size_t NUM_REPLY_MODES = 3;


/**
 * Type of a row of reply message.
 */
struct ReplyMessage {
  ReplyMode mode;
  User user;
  Content message;

  ReplyMessage() {}
  ReplyMessage(ReplyMode mode, User const& user, Content const& message) :
      mode(mode), user(user), message(message) {}
};


/**
 * Type of an array of reply messages.
 */
typedef std::vector<ReplyMessage> ReplyMessages;


/**
 * Meta data of a document.
 */
struct DocMetaData {
  size_t id;
  size_t prev_id;
  DocTitle title;
  User author;
  Time post_time;
  Board board;

  size_t num_reply_rows[NUM_REPLY_MODES];

  DocMetaData() {}
  DocMetaData(size_t id,
              size_t prev_id,
              DocTitle const& title,
              User const& author,
              Time const& post_time,
              Board const& board,
              size_t num_reply_rows[NUM_REPLY_MODES]) :
      id(id), prev_id(prev_id),
      title(title), author(author), post_time(post_time), board(board) {
    memcpy(this->num_reply_rows, num_reply_rows, sizeof(this->num_reply_rows));
  }
};


/**
 * Real document content data.
 */
struct DocRealData {
  Content content;
  ReplyMessages reply_messages;

  DocRealData() {}
  DocRealData(Content const& content, ReplyMessages const& reply_messages) :
      content(content), reply_messages(reply_messages) {}
};

}  // namespace types

}  // namespace protocol

#endif  // ARTICLE_ANALYSIS_PROTOCOL_TYPES_H_
