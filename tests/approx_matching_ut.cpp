/*
 * approx_matching_ut.cpp --
 *
 * Copyright (c) 2019 YANDEX LLC, Karina Usmanova <usmanova.karin@yandex.ru>
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
#include "common.h"

SIMPLE_UNIT_TEST_SUITE(ApproxMatchingTest) {
	Pire::Fsm BuildFsm(const char *str)
	{
		Pire::Lexer lexer;
		TVector<wchar32> ucs4;

		lexer.Encoding().FromLocal(str, str + strlen(str), std::back_inserter(ucs4));
		lexer.Assign(ucs4.begin(), ucs4.end());
		return lexer.Parse();
	}

	SIMPLE_UNIT_TEST(Simple) {
		auto fsm = BuildFsm("^ab$");

		SCANNER2(fsm, 1) {
			ACCEPTS("ab");
			ACCEPTS("ax");
			ACCEPTS("xb");
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("xab");
			ACCEPTS("axb");
			ACCEPTS("abx");
			ACCEPTS("aab");
			DENIES("xy");
			DENIES("abcd");
			DENIES("xabx");
			DENIES("");
		}

		fsm = BuildFsm("^ab$");
		SCANNER2(fsm, 2) {
			ACCEPTS("ab");
			ACCEPTS("xy");
			ACCEPTS("");
			ACCEPTS("axbx");
			DENIES("xxabx");
			DENIES("xbxxx");
		}
	}

	SIMPLE_UNIT_TEST(SpecialSymbols) {
		auto fsm = BuildFsm("^.*ab$");

		SCANNER2(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("ab");
			ACCEPTS("xxxxab");
			ACCEPTS("xxxxabab");
			DENIES("xxxx");
			DENIES("abxxxx");
		}

		fsm = BuildFsm("^[a-c]$");
		SCANNER2(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("c");
			ACCEPTS("/");
			ACCEPTS("");
			ACCEPTS("ax");
			DENIES("xx");
			DENIES("abc");
		}

		fsm = BuildFsm("^x{4}$");
		SCANNER2(fsm, 2) {
			DENIES ("x");
			ACCEPTS("xx");
			ACCEPTS("xxx");
			ACCEPTS("xxxx");
			ACCEPTS("xxxxx");
			ACCEPTS("xxxxxx");
			DENIES ("xxxxxxx");
			ACCEPTS("xxyy");
			ACCEPTS("xxyyx");
			ACCEPTS("xxxxyz");
			DENIES("xyyy");
		}

		fsm = BuildFsm("^(a|b)$");
		SCANNER2(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("x");
			ACCEPTS("");
			ACCEPTS("ax");
			DENIES("abc");
			DENIES("xx");
		}

		fsm = BuildFsm("^(ab|cd)$");
		SCANNER2(fsm, 1) {
			ACCEPTS("ab");
			ACCEPTS("cd");
			ACCEPTS("ax");
			ACCEPTS("xd");
			ACCEPTS("abx");
			ACCEPTS("a");
			DENIES("abcd");
			DENIES("xx");
			DENIES("");
		}

		fsm = BuildFsm("^[a-c]{3}$");
		SCANNER2(fsm, 2) {
			ACCEPTS("abc");
			ACCEPTS("aaa");
			ACCEPTS("a");
			ACCEPTS("ax");
			ACCEPTS("abxcx");
			DENIES("x");
			DENIES("");
			DENIES("xaxx");
		}

		fsm = BuildFsm("^\\x{61}$");
		SCANNER2(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("x");
			ACCEPTS("");
			ACCEPTS("ax");
			DENIES("axx");
			DENIES("xx");
		}

	}

	SIMPLE_UNIT_TEST(TestSurrounded) {
		auto fsm = BuildFsm("abc").Surround();

		SCANNER2(fsm, 1) {
			ACCEPTS("abc");
			ACCEPTS("abcd");
			ACCEPTS("xabcx");
			ACCEPTS("xabxx");
			DENIES("a");
			DENIES("xaxxxx");
		}

		fsm = BuildFsm("^abc$").Surround();
		SCANNER2(fsm, 1) {
			ACCEPTS("abc");
			ACCEPTS("abcx");
			ACCEPTS("xabc");
			ACCEPTS("axc");
			DENIES("xabx");
			DENIES("axx");
		}
	}

	SIMPLE_UNIT_TEST(GlueFsm) {
		auto fsm = BuildFsm("^a$") | BuildFsm("^b$");
		SCANNER2(fsm, 1) {
			ACCEPTS("");
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("x");
			ACCEPTS("ab");
			DENIES("abb");
		}

		fsm = BuildFsm("^[a-b]$") | BuildFsm("^c{2}$");

		SCANNER2(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("cc");
			ACCEPTS("x");
			ACCEPTS("xa");
			ACCEPTS("c");
			ACCEPTS("xc");
			ACCEPTS("cxc");
			ACCEPTS("");
		}
	}
}
