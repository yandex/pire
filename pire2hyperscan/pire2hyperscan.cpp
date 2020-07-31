/*
 * pire2hyperscan.cpp -- convert Pire regex to Hyperscan regex
 *
 * Copyright (c) 2007-2010, Dmitry Prokoptsev <dprokoptsev@gmail.com>,
 *                          Alexander Gololobov <agololobov@gmail.com>
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

#include "pire2hyperscan.h"

#include <pire/encoding.h> // ToUtf8
#include <pire/pire.h>
#include <pire/re_parser.h> // YRE_AND, YRE_NOT

#include <sstream>
#include <stdexcept>


namespace Pire {

    struct TCountedTerm {
        Term MainTerm;
        int MinCount;
        int MaxCount;

        TCountedTerm(const Term term)
            : MainTerm(term)
            , MinCount(1)
            , MaxCount(1)
        {
        }
    };


    using TCharacterRange = Term::CharacterRange;


    static ystring ToUtf8(wchar32 letter32) {
        return Encodings::Utf8().ToLocal(letter32);
    }


    static bool NeedBrackets(const TCharacterRange& range) {
        if (range.second) {
            return true;
        }
        if (range.first.size() != 1) {
            return true;
        }
        auto wideLetter = *range.first.begin();
        if (wideLetter.size() != 1) {
            return true; // will throw NHyperscan::TCompileException
        }
        ystring letter = ToUtf8(wideLetter[0]);
        if (letter.size() != 1) {
            return true;
        }
        return !isalnum(letter[0]);
    }


    static bool NeedEscape(const ystring& ch) {
        return ch == "-" || ch == "[" || ch == "]" || ch == "\\" || ch == "^";
    }


    ystring PireLexer2Hyperscan(Lexer& lexer) {

        // Step 1. Turn lexer into a vector of terms
        yvector<TCountedTerm> terms;
        for (Term term = lexer.Lex(); term.Type() != 0; term = lexer.Lex()) {
            if (term.Type() == YRE_COUNT) {
                using TRepetitionCount = Term::RepetitionCount;
                const TRepetitionCount& value = term.Value().As<TRepetitionCount>();
                YASSERT(!terms.empty());
                terms.back().MinCount = value.first;
                terms.back().MaxCount = value.second;
            } else {
                terms.push_back(term);
            }
        }

        // Step 2. Turn the vector of terms back to regex string.
        std::stringstream result;
        for (size_t i = 0; i < terms.size(); i++) {
            const TCountedTerm term = terms[i];

            // If first term is [^...], it matches text begin in Pire
            // Example: /[^4]submit/
            // The following conditions are required to match text begin/end:
            // 1. the term is first or last
            // 2. length can be 1, so it could be begin/end mark in Pire
            bool mayNeedMask = (term.MinCount == 1);
            // terms.size() > 1; https://github.com/01org/hyperscan/issues/25
            auto fixBefore = [&]() {
                if (mayNeedMask) {
                    if (i == 0) {
                        result << "(^|";
                    } else if (i == terms.size() - 1) {
                        result << "($|";
                    }
                }
            };
            auto fixAfter = [&]() {
                if (mayNeedMask) {
                    if (i == 0) {
                        result << ")";
                    } else if (i == terms.size() - 1) {
                        result << ")";
                    }
                }
            };

            auto printCount = [&]() {
                if (term.MinCount != 1 || term.MaxCount != 1) {
                    result << '{' << term.MinCount << ',';
                    if (term.MaxCount != Consts::Inf) {
                        result << term.MaxCount;
                    }
                    result << '}';
                }
            };

            int type = term.MainTerm.Type();
            if (type == YRE_LETTERS) {
                if (!term.MainTerm.Value().IsA<TCharacterRange>()) {
                    throw NHyperscan::TCompileException();
                }
                const TCharacterRange& value = term.MainTerm.Value().As<TCharacterRange>();
                if (value.second) {
                    fixBefore();
                }
                if (NeedBrackets(value)) {
                    result << '[';
                }
                if (value.second) {
                    result << '^';
                }
                for (const auto& str : value.first) {
                    if (str.size() != 1) {
                        // members of [...] must be 1-letter
                        throw NHyperscan::TCompileException();
                    }
                    ystring utf8String = ToUtf8(str[0]);
                    if (NeedEscape(utf8String)) {
                        result << '\\';
                    }
                    result << utf8String;
                }
                if (NeedBrackets(value)) {
                    result << ']';
                }
                printCount();
                if (value.second) {
                    fixAfter();
                }
            } else if (type == YRE_DOT) {
                fixBefore();
                result << '.';
                printCount();
                fixAfter();
            } else if (type == YRE_AND) {
                throw NHyperscan::TCompileException();
            } else if (type == YRE_NOT) {
                throw NHyperscan::TCompileException();
            } else if (type == '(') {
                result << '(';
            } else if (type == ')') {
                result << ')';
                printCount();
            } else if (type == '|') {
                result << '|';
            } else if (type == '^') {
                result << '^';
            } else if (type == '$') {
                result << '$';
            } else {
                std::stringstream errorMessage;
                errorMessage << "Unknown term type: ";
                errorMessage << type;
                throw std::logic_error(errorMessage.str());
            }
        }
        return result.str();
    }


    ystring PireRegex2Hyperscan(const ystring& regex) {
        yvector<wchar32> ucs4;
        Encodings::Utf8().FromLocal(
            regex.data(),
            regex.data() + regex.size(),
            std::back_inserter(ucs4)
        );
        Lexer lexer(ucs4.begin(), ucs4.end());
        lexer.AddFeature(Features::AndNotSupport());
        return PireLexer2Hyperscan(lexer);
    }

}
