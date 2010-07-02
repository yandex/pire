#include <stdexcept>
#include <utility>
#include <assert.h>
#include "stub/defaults.h"
#include "stub/utf8.h"
#include "stub/singleton.h"
#include "encoding.h"
#include "fsm.h"


namespace Pire {

namespace {

	class Latin1: public Encoding {
	public:
		wchar32 FromLocal(const char*& begin, const char* end) const
		{
			if (begin == end)
				throw Error("EOF reached in Pire::Latin1::fromLocal()");
			else if (*begin < 0)
				throw Error("Pire::Latin1::fromLocal(): wrong character encountered (>=0x80)");
			else
				return (wchar32) *begin++;
		}

		std::string ToLocal(wchar32 ch) const
		{
			if (ch < 0x80)
				return std::string(1, (char) ch);
			else
				return std::string();
		}

		void AppendDot(Fsm& fsm) const { fsm.AppendDot(); }
	};
	
	namespace UtfRanges {

		static const size_t MaxLen = 4;
		std::pair<size_t, size_t> First[MaxLen] = {
			std::make_pair(0x01, 0x80),
			std::make_pair(0xC0, 0xE0),
			std::make_pair(0xE0, 0xF0),
			std::make_pair(0xF0, 0xF8)
		};
		std::pair<size_t, size_t> Next(0x80, 0xC0);
	}

static const Latin1 latin1;


	class Utf8: public Encoding {
	public:
		wchar32 FromLocal(const char*& begin, const char* end) const
		{
			wchar32 rune;
			size_t len;
			if (utf8_read_rune(rune, len, reinterpret_cast<const unsigned char*>(begin), reinterpret_cast<const unsigned char*>(end)) != RECODE_OK)
				throw Error("Error reading UTF8 sequence");
			begin += len;
			return rune;
		}

		std::string ToLocal(wchar32 c) const
		{
			std::string ret(utf8_rune_len_by_ucs(c), ' ');
			size_t len;
			unsigned char* p = (unsigned char*) &*ret.begin();
			if (utf8_put_rune(c, len, p, p + ret.size()) != RECODE_OK)
				assert(!"Pire::UTF8::toLocal(): Internal error");
			return ret;
		}

		void AppendDot(Fsm& fsm) const
		{
			size_t last = fsm.Resize(fsm.Size() + UtfRanges::MaxLen);
			for (size_t i = 0; i < UtfRanges::MaxLen; ++i)
				for (size_t letter = UtfRanges::First[i].first; letter < UtfRanges::First[i].second; ++letter)
					fsm.ConnectFinal(fsm.Size() - i - 1, letter);
			for (size_t i = 0; i < UtfRanges::MaxLen - 1; ++i)
				for (size_t letter = UtfRanges::Next.first; letter < UtfRanges::Next.second; ++letter)
					fsm.Connect(last + i, last + i + 1, letter);
			fsm.ClearFinal();
			fsm.SetFinal(fsm.Size() - 1, true);
			fsm.SetIsDetermined(false);
		}
	};

static const Utf8 utf8;
}

namespace Encodings {

	const Encoding& Utf8() { return utf8; }
	const Encoding& Latin1() { return latin1; }

}

}
