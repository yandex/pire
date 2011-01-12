#include "easy.h"

namespace Pire {

const Option<const Encoding&> UTF8(&Pire::Encodings::Utf8);
const Option<const Encoding&> LATIN1(&Pire::Encodings::Latin1);
	
const Option<Feature*> I(&Pire::Features::CaseInsensitive);
const Option<Feature*> ANDNOT(&Pire::Features::AndNotSupport);

}
