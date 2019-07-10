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

		APPROXIMATE_SCANNER(fsm, 1) {
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
		APPROXIMATE_SCANNER(fsm, 2) {
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

		APPROXIMATE_SCANNER(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("ab");
			ACCEPTS("xxxxab");
			ACCEPTS("xxxxabab");
			DENIES("xxxx");
			DENIES("abxxxx");
		}

		fsm = BuildFsm("^[a-c]$");
		APPROXIMATE_SCANNER(fsm, 1) {
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
		APPROXIMATE_SCANNER(fsm, 2) {
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
		APPROXIMATE_SCANNER(fsm, 1) {
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("x");
			ACCEPTS("");
			ACCEPTS("ax");
			DENIES("abc");
			DENIES("xx");
		}

		fsm = BuildFsm("^(ab|cd)$");
		APPROXIMATE_SCANNER(fsm, 1) {
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
		APPROXIMATE_SCANNER(fsm, 2) {
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
		APPROXIMATE_SCANNER(fsm, 1) {
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

		APPROXIMATE_SCANNER(fsm, 1) {
			ACCEPTS("abc");
			ACCEPTS("abcd");
			ACCEPTS("xabcx");
			ACCEPTS("xabxx");
			DENIES("a");
			DENIES("xaxxxx");
		}

		fsm = BuildFsm("^abc$").Surround();
		APPROXIMATE_SCANNER(fsm, 1) {
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
		APPROXIMATE_SCANNER(fsm, 1) {
			ACCEPTS("");
			ACCEPTS("a");
			ACCEPTS("b");
			ACCEPTS("x");
			ACCEPTS("ab");
			DENIES("abb");
		}

		fsm = BuildFsm("^[a-b]$") | BuildFsm("^c{2}$");

		APPROXIMATE_SCANNER(fsm, 1) {
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

	enum MutateOperation {
		Substitute = 0,
		Delete = 1,
		Insert = 2
	};
	
	ystring ChangeText(const ystring& text, int operation, int posLeft, int posRight = -1)
	{
		auto changedText = text;
		switch (operation) {
			case MutateOperation::Substitute:
				if (posRight >= 0) {
					changedText[posRight] = 'x';
				}
				changedText[posLeft] = 'x';
				break;
			case MutateOperation::Delete:
				if (posRight >= 0) {
					changedText.erase(posRight, 1);
				}
				changedText.erase(posLeft, 1);
				break;
			case MutateOperation::Insert:
				if (posRight >= 0) {
					changedText.insert(posRight + 1, 1, 'x');
				}
				changedText.insert(posLeft, 1, 'x');
				break;
		}

		return changedText;
	}

	SIMPLE_UNIT_TEST(StressTest) {
		ystring text;
		for (size_t letter = 0; letter < 10; ++letter) {
			text += ystring(3, letter + 'a');
		}
		const ystring regexp = "^" + text + "$";
		auto fsm = BuildFsm(regexp.data());

		APPROXIMATE_SCANNER(fsm, 1) {
			ACCEPTS(text);

			for (size_t pos = 0; pos < regexp.size() - 2; ++pos) {
				for (int operation = 0; operation < 3; ++operation) {
					auto changedText = ChangeText(text, operation, pos);
					ACCEPTS(changedText);
				}
			}
		}

		APPROXIMATE_SCANNER(fsm, 0) {
			ACCEPTS(text);

			for (size_t pos = 0; pos < regexp.size() - 2; ++pos) {
				for (int operation = 0; operation < 3; ++operation) {
					auto changedText = ChangeText(text, operation, pos);
					DENIES(changedText);
				}
			}
		}

		APPROXIMATE_SCANNER(fsm, 2) {
			ACCEPTS(text);

			for (size_t posLeft = 0; posLeft < text.size() / 2; ++posLeft) {
				size_t posRight = text.size() - posLeft - 1;
				for (int operation = 0; operation < 3; ++operation) {
					auto changedText = ChangeText(text, operation, posLeft, posRight);
					ACCEPTS(changedText);
				}
			}
		}

		APPROXIMATE_SCANNER(fsm, 1) {
			ACCEPTS(text);

			for (size_t posLeft = 0; posLeft < text.size() / 2; ++posLeft) {
				size_t posRight = text.size() - posLeft - 1;

				for (int operation = 0; operation < 3; ++operation) {
					auto changedText = ChangeText(text, operation, posLeft, posRight);
					DENIES(changedText);
				}
			}
		}
	}
}
