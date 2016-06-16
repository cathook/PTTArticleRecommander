#ifndef ARTICLE_ANALYSIS_PROTOCOL_I_LOADALBE_H_
#define ARTICLE_ANALYSIS_PROTOCOL_I_LOADALBE_H_


#include <string>


namespace protocol {


/**
 * An interface which an loadable class/struct should implement.
 */
class ILoadable {
 public:
  virtual ~ILoadable() {}

  /**
   * Loads from the byte array.
   *
   * @param [in] buf The bype array.
   * @param [in,out] offset Offset of the start byte.
   * @return `true` if no error occured.
   */
  virtual bool Load(std::string const& buf, size_t* offset) = 0;

 protected:
  ILoadable() {}
};

}  // namespace protocol

#endif  // ARTICLE_ANALYSIS_PROTOCOL_I_LOADALBE_H_
