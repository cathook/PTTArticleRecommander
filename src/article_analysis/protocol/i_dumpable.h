#ifndef ARTICLE_ANALYSIS_PROTOCOL_I_DUMPALBE_H_
#define ARTICLE_ANALYSIS_PROTOCOL_I_DUMPALBE_H_


#include <string>


namespace protocol {


/**
 * An interface which a dumpable class/struct should implement.
 */
class IDumpable {
 public:
  virtual ~IDumpable() {}

  /**
   * Dumps into byte array.
   *
   * @return A byte array.
   */
  virtual std::string Dump() const = 0;

 protected:
  IDumpable() {}
};

}  // namespace protocol

#endif  // ARTICLE_ANALYSIS_PROTOCOL_I_DUMPALBE_H_
