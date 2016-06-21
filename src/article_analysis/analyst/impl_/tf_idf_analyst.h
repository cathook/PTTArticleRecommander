#include "analyst/i_analyst.h"
#include "miner/miner.h"

namespace analyst {

namespace impl_ {


/**
 * Just an fool analyst, it will return nothing.
 */
class TfIdfAnalyst : public IAnalyst {
	public:
		TfIdfAnalyst(miner::Miner *miner);
		DocRelInfo GetDocRelInfo(DocIdentity const& id) const override final;
	private:
		std::vector<std::vector<int> > recommended_file;
		std::vector<bool>file_calculated;
};

}  // namespace impl_

}  // namespace analyst
