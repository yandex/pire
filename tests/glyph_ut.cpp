#include <pire.h>
#include <extra/glyphs.h>
#include "stub/cppunit.h"
#include "common.h"

SIMPLE_UNIT_TEST_SUITE(Glyphs) {

	Pire::Fsm ParseFsm(const char* regexp)
	{
		yvector<Pire::wchar32> ucs4;
		Pire::Encodings::Utf8().FromLocal(regexp, regexp + strlen(regexp), std::back_inserter(ucs4));
		return Pire::Lexer(ucs4).SetEncoding(Pire::Encodings::Utf8()).AddFeature(Pire::Features::GlueSimilarGlyphs()).Parse().Surround();
	}

#define NOGL_REGEXP(str) REGEXP2(str, "u")
#define GL_REGEXP(str) SCANNER(ParseFsm(str))

	SIMPLE_UNIT_TEST(Glyphs)
	{
		NOGL_REGEXP("regexp") {
			ACCEPTS("regexp");
			DENIES("r\xD0\xB5g\xD0\xB5\xD1\x85\xD1\x80");
		}

		GL_REGEXP("regexp") {
			ACCEPTS("regexp");
			ACCEPTS("r\xD0\xB5g\xD0\xB5\xD1\x85\xD1\x80");
		}

		NOGL_REGEXP("r\xD0\xB5g\xD0\xB5\xD1\x85\xD1\x80") {
			DENIES("regexp");
			ACCEPTS("r\xD0\xB5g\xD0\xB5\xD1\x85\xD1\x80");
		}

		GL_REGEXP("r\xD0\xB5g\xD0\xB5\xD1\x85\xD1\x80") {
			ACCEPTS("regexp");
			ACCEPTS("r\xD0\xB5g\xD0\xB5\xD1\x85\xD1\x80");
		}
	}
}
