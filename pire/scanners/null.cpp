#include "../fsm.h"
#include "multi.h"
#include "simple.h"
#include "slow.h"
#include "loaded.h"

namespace Pire {
	
namespace Impl {
	class NullLoadedScanner: public Pire::LoadedScanner {
	public:
		explicit NullLoadedScanner(Pire::Fsm& fsm)
		{
			fsm.Canonize();
			Init(fsm.Size(), fsm.Letters(), fsm.Initial());
			BuildScanner(fsm, *this);
		}		
	};
}

const SimpleScanner SimpleScanner::m_null = Fsm::MakeFalse().Compile<SimpleScanner>();
const SlowScanner   SlowScanner  ::m_null = Fsm::MakeFalse().Compile<SlowScanner>();
const LoadedScanner LoadedScanner::m_null = Fsm::MakeFalse().Compile<Impl::NullLoadedScanner>();

}
