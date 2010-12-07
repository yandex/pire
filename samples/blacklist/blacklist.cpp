/*
 * blacklist.cpp -- an example of Pire serialization
 *                  and complex regexp parsing
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
 * Suppose you have a blacklist of domains you do not like for any reason
 * and you want your application to recognize URLs belonging to those
 * domains. The problems are:
 *    - the blacklist can be relatively large (say, thousands of domains);
 *    - check needs to be performed quickly;
 *    - you don't want to write any URL parsing routine.
 *
 * If in Perl, you might just write something like
 *    if ($url =~ m!^([a-z]+://)?([A-Za-z0-9\-]+\.)*(domain1|domain2|...)(/.*)?$!) { ... }
 *                 #  (scheme)    (subdomain)       (blacklist)            (local path)
 * Unfortunately, you're in C++ and do not have those fancy regexps; in fact,
 * you're not satisfied with thier speed anyway (they won't survive a thousand of patterns).
 *
 * Pire offers a solution almost as easy as specified above. First, you join all your
 * patterns in a single huge regexp, compile it into Scanner and save it in a file
 * (see Generate()). Next, you mmap() this file, load a scanner (see Use()) and can
 * easily check whether given string matches your huge regexp or not (see Filter()).
 *
 * In this example, blacklist items are treated as fixed strings (if you read 'blabla.com',
 * you might expect dot to mean precisely one dot, not any symbol), but there's nothing
 * to prevent you from reading arbitrary regexps.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <pire/pire.h>
#include "../../tools/common/filemap.h"

void Usage()
{
    std::cerr << "Usage: blacklist -g -d <filename>  -- reads series of domains and makes a dictionary\n"
              << "       blacklist -u -d <filename>  -- reads the dictionary and passes matching URLs to stdout\n"
              << std::endl;
    exit(1);
}

void Generate(const std::string& outputFile)
{
    Pire::Fsm re = Pire::Fsm::MakeFalse();
    std::string domain;
    while (std::cin >> domain)
        re |= Pire::Fsm().Append(domain);
    re = Pire::Lexer("^([a-z]+://)?([A-Za-z0-9\\-]+\\.)*").Parse() + re + Pire::Lexer("(/.*)?$").Parse();
    
    std::fstream ofs(outputFile.c_str(), std::ios::out | std::ios::binary);
    Pire::Scanner(re).Save(&ofs);
}

void Filter(const Pire::Scanner& sc)
{
    std::string url;
    while (std::cin >> url) {
        if (Pire::Runner(sc).Begin().Run(url).End())
            std::cout << url << std::endl;
    }
}

void Use(const std::string& filename)
{   
	FileMmap fileMmap(filename.c_str());
    
    Pire::Scanner sc;
    sc.Mmap(fileMmap.Begin(), fileMmap.Size());
    Filter(sc);
}

#ifndef _WIN32
#include <libgen.h>
#else
const char* basename(const char* name)
{
	return name;
}
#endif

int main(int argc, char** argv)
{
    std::string myname = basename(argv[0]);
    try {
        std::ios_base::sync_with_stdio(false);

		std::string filename;
        void (*mode)(const std::string&) = 0;
        for (--argc, ++argv; argc; --argc, ++argv) {
            if (!strcmp(*argv, "-d") && argc >= 2) {
                filename = argv[1];
				++argv; --argc;
            } else if (!strcmp(*argv, "-g")) {
                mode = Generate;
            } else if (!strcmp(*argv, "-u")) {
                mode = Use;
            } else
                Usage();
        }
        if (filename.empty() || !mode)
            Usage();
        (*mode)(filename);
    }
    catch (std::exception& e) {
        std::cerr << myname << ": " << e.what() << std::endl;
        return 1;
    }
}
