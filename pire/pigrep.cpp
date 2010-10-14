/*
 * pigrep.cpp -- a verbose Pire runner
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


#include <string>
#include <stdexcept>
#include <iostream>
#include <pire.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

using Pire::Cdbg;
using Pire::Endl;
using Pire::ystring;

void Run(const Pire::Scanner& scanner, Pire::Scanner::State& state, const ystring& data)
{
	PIRE_IFDEBUG(Cdbg << "---run---" << Endl);
	Pire::Run(scanner, state, data.c_str(), data.c_str() + data.size());
}

bool ReadLine(FILE* f, ystring& str)
{
	int ch;
	str.clear();
	while ((ch = getc(f)) != EOF && ch != '\n')
		str.push_back((char) ch);
	return !str.empty() || ch != EOF;
}

int main(int argc, char** argv)
{
	try {
		if (argc < 2)
			throw Pire::Error("Usage: pigrep <regexp> <options>");
		Pire::Lexer lexer(argv[1], argv[1] + strlen(argv[1]));
		bool surround = false;
		if (argc >= 3)
			for (const char* option = argv[2]; *option; ++option)
				if (*option == 'i')
					lexer.AddFeature(Pire::Features::CaseInsensitive());
				else if (*option == 'u')
					lexer.SetEncoding(Pire::Encodings::Utf8());
				else if (*option == 's')
					surround = true;
				else if (*option == 'a')
					lexer.AddFeature(Pire::Features::AndNotSupport());
				else
					throw Pire::Error("Unknown option");
		Pire::Fsm fsm = lexer.Parse();
		if (surround)
			fsm.Surround();
		Pire::Scanner scanner(fsm);

		ystring str;
		while (ReadLine(stdin, str)) {
			PIRE_IFDEBUG(Cdbg << "---run---" << Endl);
			if ((surround && Pire::Runner(scanner).Begin().Run(str).End())
				|| (!surround && Pire::Runner(scanner).Run(str)))
			{
				std::cout << str << std::endl;
			}
		}

		return 0;
	} catch (std::exception& e) {
		std::cerr << "pigrep: " << e.what() << std::endl;
		return 1;
	}
}
