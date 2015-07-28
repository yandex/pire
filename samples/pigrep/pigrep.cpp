/*
 * pigrep.cpp -- an example of a grep-like tool
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

/**
 * This example just prints lines matching given regexp, as usual
 * grep(1) does. Since it is just an example, it does not contain
 * all those recursive directories traversal, multiple output formats
 * and other stuff which is unnecessary for understanding things.
 */

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <pire/pire.h>

void GrepStream(std::istream& stream, const Pire::Scanner& sc, const std::string& prefix)
{
    std::string line;
    while (getline(stream, line)) {
        if (Pire::Runner(sc).Begin().Run(line).End())
            std::cout << prefix << line << std::endl;
    }
}

void Usage()
{
    std::cerr << "Usage: pigrep [-i] [-u] [-x] [-e pattern | pattern] [file [file2...]]\n"
              << "  -i    Be case insensitive\n"
              << "  -u    Interpret input sequence and pattern as UTF-8 strings\n"
              << "  -x    Enable extended syntax (\"re1&re2\" for conjunction and \"~re\" for negation)\n"
              << "  -e    Specify regexp pattern (useful if it begins with a dash)\n"
              << "When no files are given, stdin is examined." << std::endl;
    exit(1);
}

int main(int argc, char** argv)
{
	try {
        Pire::Lexer lexer;
        std::string patternstr(1, 0);
	for (--argc, ++argv; argc; --argc, ++argv) {
		if (!strcmp(*argv, "-i")) {
			lexer.AddFeature(Pire::Features::CaseInsensitive());
		} else if (!strcmp(*argv, "-u")) {
			lexer.SetEncoding(Pire::Encodings::Utf8());
		} else if (!strcmp(*argv, "-x")) {
			lexer.AddFeature(Pire::Features::AndNotSupport());
		} else if (!strcmp(*argv, "-e") && argc >= 2 && !patternstr.empty() && !patternstr[0]) {
			patternstr = argv[1];
			++argv; --argc;
		} else if (argv[0] != 0 && argv[0][0] == '-') {
			break;
		} else if (!patternstr.empty() && !patternstr[0]) {
			patternstr = argv[0];
		} else {
			break;
		}
	}
        if (!patternstr.empty() && !patternstr[0]) {
                Usage();
        }
        std::vector<Pire::wchar32> pattern;
        lexer.Encoding().FromLocal(patternstr.c_str(), patternstr.c_str() + patternstr.size(), std::back_inserter(pattern));
        lexer.Assign(pattern.begin(), pattern.end());
        Pire::Scanner sc = lexer.Parse().Surround().Compile<Pire::Scanner>();
        
        std::ios_base::sync_with_stdio(false);
        if (argc == 0)
            GrepStream(std::cin, sc, std::string());
        else {
            for (char** filename = argv; filename != argv + argc; ++filename) {
                std::string prefix = "(stdin)";
                std::istream* s = &std::cin;
                std::ifstream fs;

                if (*filename != std::string("-")) {
                    fs.open(*filename);
                    if (!fs)
                        throw std::runtime_error("cannot open file " + std::string(argv[0]));
                    prefix = *filename;
                    s = &fs;
                }

                GrepStream(*s, sc, argc == 1 ? "" : (prefix + ": "));
            }
        }

		return 0;
	} catch (std::exception& e) {
		std::cerr << "pigrep: " << e.what() << std::endl;
		return 1;
	}
}
