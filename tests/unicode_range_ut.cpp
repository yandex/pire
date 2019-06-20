/*
 * unicode_range_ut.cpp --
 *
 * Copyright (c) 2019 YANDEX LLC
 * Author: Karina Usmanova <usmanova.karin@yandex.ru>
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
#include <extra/unicode_range.h>
#include "stub/cppunit.h"
#include "common.h"

SIMPLE_UNIT_TEST_SUITE(UnicodeRangeTest) {

    Pire::Fsm ParseRegexpWithUnicodeRange(const char* regexp)
    {
        TVector<wchar32> ucs4;
        Pire::Encodings::Utf8().FromLocal(regexp, regexp + strlen(regexp), std::back_inserter(ucs4));
        auto feature = Pire::Features::UnicodeRange();
        return Pire::Lexer(ucs4).SetEncoding(Pire::Encodings::Utf8()).AddFeature(feature).Parse().Surround();
    }

    SIMPLE_UNIT_TEST(OneCharacter)
    {
        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x{61}]")) {
            ACCEPTS("a");
            ACCEPTS("bac");
            DENIES("test");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x3f]")) {
            ACCEPTS("?");
            ACCEPTS("test?");
            DENIES("test");
        }
    }

    SIMPLE_UNIT_TEST(CharacterRange) {
        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x{61}\\x62\\x{3f}\\x26]")) {
            ACCEPTS("a");
            ACCEPTS("b");
            ACCEPTS("?");
            ACCEPTS("acd");
            ACCEPTS("bcd");
            ACCEPTS("cd?");
            ACCEPTS("ab?");
            DENIES("cd");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x{61}-\\x{63}]")) {
            ACCEPTS("a");
            ACCEPTS("b");
            ACCEPTS("c");
            ACCEPTS("qwertya");
            DENIES("d");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x61-\\x63]")) {
            ACCEPTS("a");
            ACCEPTS("b");
            ACCEPTS("c");
            ACCEPTS("qwertya");
            DENIES("d");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x26\\x{61}-\\x{63}\\x{3f}]")) {
            ACCEPTS("&");
            ACCEPTS("a");
            ACCEPTS("b");
            ACCEPTS("c");
            ACCEPTS("?");
            ACCEPTS("ade");
            ACCEPTS("abc?");
            DENIES("d");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x{41}-\\x{43}\\x{61}-\\x{63}]")) {
            ACCEPTS("a");
            ACCEPTS("b");
            ACCEPTS("c");
            ACCEPTS("A");
            ACCEPTS("B");
            ACCEPTS("C");
            DENIES("d");
            DENIES("D");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[\\x{41}-\\x{42}]\\r[\\x{61}-\\x{62}]")) {
            ACCEPTS("Aa");
            ACCEPTS("Ab");
            ACCEPTS("Ba");
            ACCEPTS("Bb");
            DENIES("a");
            DENIES("b");
            DENIES("A");
            DENIES("B");
            DENIES("ab");
            DENIES("AB");
            DENIES("Ca");
        }
    }

    SIMPLE_UNIT_TEST(ExcludeCharacters) {
        SCANNER(ParseRegexpWithUnicodeRange("\\r[^\\x{61}]")) {
            ACCEPTS("b");
            ACCEPTS("c");
            ACCEPTS("aba");
            DENIES("a");
            DENIES("aaa");
        }

        SCANNER(ParseRegexpWithUnicodeRange("\\r[^\\x{61}-x{7a}]")) {
            ACCEPTS("A");
            ACCEPTS("123");
            ACCEPTS("acb1");
            DENIES("a");
            DENIES("abcxyz");
        }
    }


    SIMPLE_UNIT_TEST(Compiling)
    {
        UNIT_ASSERT(HasError("\\r[\\x41", ParseRegexpWithUnicodeRange));
        UNIT_ASSERT(HasError("\\r[\\xfq]", ParseRegexpWithUnicodeRange));

        UNIT_ASSERT(HasError("\\r[\\x{01}-]", ParseRegexpWithUnicodeRange));
        UNIT_ASSERT(HasError("\\r[-\\x{01}]", ParseRegexpWithUnicodeRange));
        UNIT_ASSERT(HasError("\\r[^^\\x{01}]", ParseRegexpWithUnicodeRange));

        UNIT_ASSERT(!HasError("\\r[\\x{10FFFF}]", ParseRegexpWithUnicodeRange));
        UNIT_ASSERT(!HasError("\\r[\\x{00}]", ParseRegexpWithUnicodeRange));
        UNIT_ASSERT(!HasError("\\r[\\x{abc}-\\x{FFF}]", ParseRegexpWithUnicodeRange));

        UNIT_ASSERT(!HasError("\\r[^\\xFF]", ParseRegexpWithUnicodeRange));
        UNIT_ASSERT(!HasError("\\r[^\\x{FF}-\\x{FF0}]", ParseRegexpWithUnicodeRange));
    }
}
