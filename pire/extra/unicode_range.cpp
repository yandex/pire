/*
 * unicode_range.cpp -- implementation of the UnicodeRange feature.
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


#include "unicode_range.h"
#include "read_unicode.h"

#include "re_lexer.h"

namespace Pire {

namespace {
    class UnicodeRangeImpl : public UnicodeReader {
    public:
        bool Accepts(wchar32 c) const {
            return c == (Control | 'r');
        }

        Term Lex() {
            GetChar();
            wchar32 ch = GetChar();

            if (ch != '[') {
                Error("Pire::UnicodeRangeImpl::Lex(): wrong start of sequence: expected \"[\"");
            }
            ch = GetChar();
            Term::CharacterRange range;

            if (ch == '^') {
                range.second = true;
                ch = GetChar();
            }

            for (; ch != End && ch != ']'; ch = GetChar()) {
                UngetChar(ch);
                wchar32 unicode_ch = ReadUnicodeCharacter();

                if (PeekChar() == '-') {
                    GetChar();
                    wchar32 unicode_end = ReadUnicodeCharacter();

                    for (; unicode_ch <= unicode_end; ++unicode_ch) {
                        range.first.insert(Term::String(1, unicode_ch));
                    }
                } else {
                    range.first.insert(Term::String(1, unicode_ch));
                }
            }

            if (ch != ']') {
                Error("Pire::UnicodeRangeImpl::Lex(): \"\\r[...\" sequence should be closed by \"]\"");
            }

            return Term(TokenTypes::Letters, range);
        }
    };
}

namespace Features {
    Feature::Ptr UnicodeRange() { return Feature::Ptr(new UnicodeRangeImpl); }
};
}
