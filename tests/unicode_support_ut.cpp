/*
 * unicode_support_ut.cpp --
 *
 * Copyright (c) 2018 YANDEX LLC
 * Author: Andrey Logvin <andry@logvin.net>
 *
 * This file is part of Pire, the Perl Incompatible
 * Regular Expressions library.
 *
 * Pire is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pire is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser Public License
 * along with Pire.  If not, see <http://www.gnu.org/licenses>.
 */


#include <pire.h>
#include <extra/unicode_support.h>
#include "stub/cppunit.h"
#include "common.h"

SIMPLE_UNIT_TEST_SUITE(UnicodeSupport) {

    Pire::Fsm ParseFsm(const char* regexp)
    {
        TVector<wchar32> ucs4;
        Pire::Encodings::Utf8().FromLocal(regexp, regexp + strlen(regexp), std::back_inserter(ucs4));
        return Pire::Lexer(ucs4).SetEncoding(Pire::Encodings::Utf8()).AddFeature(Pire::Features::EnableUnicodeSequences()).Parse().Surround();
    }

    ystring CreateStringWithZeroSymbol(const char* str, size_t pos) {
        ystring result = str;
        Y_ASSERT(pos < result.size());
        result[pos] = '\0';
        return result;
    }

    bool HasError(const char* regexp) {
        try {
            ParseFsm(regexp);
            return false;
        } catch (Pire::Error& ex) {
            return true;
        }
    }

    SIMPLE_UNIT_TEST(ZeroSymbol)
    {
        SCANNER(ParseFsm("\\x{0}")) {
            ACCEPTS(CreateStringWithZeroSymbol("a", 0));
            ACCEPTS(CreateStringWithZeroSymbol("some text", 3));
            DENIES("string without zero");
        }

        SCANNER(ParseFsm("the\\x00middle")) {
            ACCEPTS(CreateStringWithZeroSymbol("in the middle", 6));
            DENIES(CreateStringWithZeroSymbol("in the middle", 5));
            DENIES("in the middle");
        }
    }

    SIMPLE_UNIT_TEST(SymbolsByCodes)
    {
        SCANNER(ParseFsm("\\x{41}")) {
            ACCEPTS("A");
            ACCEPTS("tAst string");
            DENIES("test string");
        }

        SCANNER(ParseFsm("\\x26abc")) {
            ACCEPTS("&abc;");
            DENIES("test &ab");
            DENIES("without");
        }
    }

    SIMPLE_UNIT_TEST(ErrorsWhileCompiling)
    {
        UNIT_ASSERT(HasError("\\x"));
        UNIT_ASSERT(HasError("\\x0"));
        UNIT_ASSERT(HasError("\\xfu"));
        UNIT_ASSERT(HasError("\\xs1"));
        UNIT_ASSERT(HasError("\\x 0"));
        UNIT_ASSERT(HasError("\\x0 "));

        UNIT_ASSERT(HasError("\\x{2A1"));
        UNIT_ASSERT(HasError("\\x{"));
        UNIT_ASSERT(HasError("\\x}"));
        UNIT_ASSERT(HasError("\\x2}"));
        UNIT_ASSERT(HasError("\\x{{3}"));
        UNIT_ASSERT(HasError("\\x{2a{5}"));

        UNIT_ASSERT(HasError("\\x{}"));
        UNIT_ASSERT(HasError("\\x{+3}"));
        UNIT_ASSERT(HasError("\\x{-3}"));
        UNIT_ASSERT(HasError("\\x{ 2F}"));
        UNIT_ASSERT(HasError("\\x{2A F}"));
        UNIT_ASSERT(HasError("\\x{2Arft}"));
        UNIT_ASSERT(HasError("\\x{110000}"));

        UNIT_ASSERT(!HasError("\\x{fB1}"));
        UNIT_ASSERT(!HasError("\\x00"));
        UNIT_ASSERT(!HasError("\\x{10FFFF}"));
    }
}
