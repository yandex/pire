#include <string.h>
#include "defaults.h"
#include "singleton.h"
#include "stl.h"

#define DECLARE_NOCOPY(klass) private: klass(const klass&); klass& operator = (const klass&);

namespace Pire {

#include "doccodes_h.h"
#include "unidata_h.h"
#include "codepage_h.h"
#include "unidata_cpp.h"

	const wchar32 BROKEN_RUNE = (wchar32) -1;

	const Recoder rcdr_to_lower[1] = {};
	const Recoder rcdr_to_upper[1] = {};
	const Recoder rcdr_to_title[1] = {};
	const CodePage *codepage_by_doccodes[1] = {};
	const CodePage *CodePageByName(const char*) { return 0; }

	NCodepagePrivate::TCodepagesMap::TCodepagesMap() {}
}
