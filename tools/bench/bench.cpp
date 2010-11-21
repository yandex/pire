/*
 * bench.cpp --
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

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <pire/pire.h>
#include "../common/filemap.h"

#ifndef _WIN32
#include <sys/time.h>

long long GetUsec()
{
	struct timeval tm;
	gettimeofday(&tm, 0);
	long long usec = tm.tv_sec * 1000000 + tm.tv_usec;
	return usec;	
}

#else // _WIN32
#include <windows.h>

long long GetUsec()
{
	FILETIME ft;
  	long long res = 0;
	GetSystemTimeAsFileTime(&ft);
	res = ft.dwHighDateTime;
	res <<= 32;
	res |= ft.dwLowDateTime;
	return res/10;
}

#endif // _WIN32

class Timer {
public:
	Timer(const std::string& msg, size_t sz): m_msg(msg), m_sz(sz) { m_tv = GetUsec(); }
    
	~Timer()
	{
		long long usec = GetUsec() - m_tv;
		float bw = m_sz *1.0 / usec;
		std::cout << m_msg << ": " << usec << " us\t" << bw << " MB/sec" <<  std::endl;
	}
    
private:
	std::string m_msg;
	long long m_tv;
	size_t m_sz;
};

typedef std::vector<Pire::Fsm> Fsms;

class ITester {
public:
	virtual ~ITester() {}
	virtual void Compile(const std::vector<Fsms>& fsms) = 0;
	virtual void Run(const char* begin, const char* end) = 0;
};

template<class Scanner>
struct Compile {
	static Scanner Do(const Fsms& fsms)
	{
		if (fsms.size() != 1)
			throw std::runtime_error("Only one regexp is allowed for this scanner");
		return Pire::Fsm(fsms[0]).Compile<Scanner>();
	}
};

template<class Relocation>
struct Compile< Pire::Impl::Scanner<Relocation> > {
	static Pire::Impl::Scanner<Relocation> Do(const Fsms& fsms)
	{
		typedef Pire::Impl::Scanner<Relocation> Sc;
		Sc sc;
		for (Fsms::const_iterator i = fsms.begin(), ie = fsms.end(); i != ie; ++i) {
			Sc tsc = Pire::Fsm(*i).Compile<Sc>();
			if (i == fsms.begin())
				tsc.Swap(sc);
			else {
				sc = Sc::Glue(sc, tsc);
				if (sc.Empty()) {
					std::ostringstream msg;
					msg << "Scanner gluing failed at regexp #" << i - fsms.begin() << " - pattern too complicated";
					throw std::runtime_error(msg.str());
				}
			}
		}
		return sc;
	}
};

template<class Scanner>
struct PrintResult {
	static void Do(const Scanner& sc, typename Scanner::State st)
	{
		if (sc.Final(st))
			std::cout << "Match" << std::endl;
		else
			std::cout << "No match" << std::endl;
	}
};

template<class Relocation>
struct PrintResult< Pire::Impl::Scanner<Relocation> > {
	typedef Pire::Impl::Scanner<Relocation> Scanner;

	static void Do(const Scanner& sc, typename Scanner::State st)
	{
		std::pair<const size_t*, const size_t*> accepted = sc.AcceptedRegexps(st);
		std::cout << "Accepted regexps:";
		for (; accepted.first != accepted.second; ++accepted.first)
			std::cout << " " << *accepted.first;
		std::cout << std::endl;
	}
};


template<class Scanner>
class Tester: public ITester {
public:
	void Compile(const std::vector<Fsms>& fsms)
	{
		if (fsms.size() != 1)
			throw std::runtime_error("Only one set of regexps is allowed for this scanner");
		sc = ::Compile<Scanner>::Do(fsms[0]);
	}
	
	void Run(const char* begin, const char* end)
	{
		PrintResult<Scanner>::Do(sc, Pire::Runner(sc).Begin().Run(begin, end).End().State());
	}
private:
	Scanner sc;
};

template<class Scanner1, class Scanner2>
class PairTester: public ITester {
	void Compile(const std::vector<Fsms>& fsms)
	{
		if (fsms.size() != 2)
			throw std::runtime_error("Only two sets of regexps are allowed for this scanner");
		sc1 = ::Compile<Scanner1>::Do(fsms[0]);
		sc2 = ::Compile<Scanner2>::Do(fsms[1]);
	}
	
	void Run(const char* begin, const char* end)
	{
		typedef Pire::ScannerPair<Scanner1, Scanner2> Pair;
		Pire::RunHelper<Pair> rh(Pair(sc1, sc2));
		rh.Begin().Run(begin, end).End();
		std::cout << "[first] "; PrintResult<Scanner1>::Do(sc1, rh.State().first);
		std::cout << "[second] "; PrintResult<Scanner2>::Do(sc2, rh.State().second);
	}
	
private:
	Scanner1 sc1;
	Scanner2 sc2;
};


class MemTester: public ITester {
public:
	void Compile(const std::vector<Fsms>&) {}
	// Just estimates memory throughput
	void Run(const char* begin, const char* end)
	{
		size_t c = 0;
		const size_t *b = (const size_t*)begin;
		const size_t *e = (const size_t*)end;
		while (b < e) { c ^= *b++; }
		std::cout << c << std::endl;
	}
};


void Main(int argc, char** argv)
{
	std::runtime_error usage("Usage: bench -f file -t {multi|nonreloc|simple|slow|null} regexp [regexp2 [-e regexp3...]] [-t <type> regexp4 [regexp5...]]");
	std::vector<Fsms> fsms;
	std::vector<std::string> types;
	std::string file;
	for (--argc, ++argv; argc; --argc, ++argv) {
		if (!strcmp(*argv, "-t") && argc >= 2) {
			types.push_back(argv[1]);
			fsms.push_back(Fsms());
			--argc, ++argv;
		} else if (!strcmp(*argv, "-f") && argc >= 2) {
			file = argv[1];
			--argc, ++argv;
		} else if (!strcmp(*argv, "-e") && argc >= 2) {
			if (fsms.empty())
				throw usage;
			fsms.back().push_back(Pire::Lexer(std::string(argv[1])).Parse().Surround());
			--argc, ++argv;
		} else {
			if (fsms.empty())
				throw usage;
			fsms.back().push_back(Pire::Lexer(std::string(*argv)).Parse().Surround());
		}
	}
	if (types.empty() || file.empty() || fsms.back().empty())
		throw usage;

	std::auto_ptr<ITester> tester;
	
	// TODO: is there a way to get out of this copypasting?
	if (types.size() == 1 && types[0] == "multi")
		tester.reset(new Tester<Pire::Scanner>);
	else if (types.size() == 1 && types[0] == "nonreloc")
		tester.reset(new Tester<Pire::NonrelocScanner>);
	else if (types.size() == 1 && types[0] == "simple")
		tester.reset(new Tester<Pire::SimpleScanner>);
	else if (types.size() == 1 && types[0] == "slow")
		tester.reset(new Tester<Pire::SlowScanner>);
	else if (types.size() == 1 && types[0] == "null")
		tester.reset(new MemTester);
	else if (types.size() == 2 && types[0] == "multi" && types[1] == "multi")
		tester.reset(new PairTester<Pire::Scanner, Pire::Scanner>);
	else if (types.size() == 2 && types[0] == "multi" && types[1] == "simple")
		tester.reset(new PairTester<Pire::Scanner, Pire::SimpleScanner>);
	else if (types.size() == 2 && types[0] == "multi" && types[1] == "nonreloc")
		tester.reset(new PairTester<Pire::Scanner, Pire::NonrelocScanner>);
	else if (types.size() == 2 && types[0] == "simple" && types[1] == "simple")
		tester.reset(new PairTester<Pire::SimpleScanner, Pire::SimpleScanner>);
	else if (types.size() == 2 && types[0] == "simple" && types[1] == "multi")
		tester.reset(new PairTester<Pire::SimpleScanner, Pire::Scanner>);
	else if (types.size() == 2 && types[0] == "simple" && types[1] == "nonreloc")
		tester.reset(new PairTester<Pire::SimpleScanner, Pire::NonrelocScanner>);
	else if (types.size() == 2 && types[0] == "nonreloc" && types[1] == "multi")
		tester.reset(new PairTester<Pire::NonrelocScanner, Pire::Scanner>);
	else if (types.size() == 2 && types[0] == "nonreloc" && types[1] == "simple")
		tester.reset(new PairTester<Pire::NonrelocScanner, Pire::SimpleScanner>);
	else if (types.size() == 2 && types[0] == "nonreloc" && types[1] == "nonreloc")
		tester.reset(new PairTester<Pire::NonrelocScanner, Pire::NonrelocScanner>);

	else
		throw usage;

	tester->Compile(fsms);
	FileMmap fmap(file.c_str());

	// Run the benchmark multiple times
	std::ostringstream stream;
	for (std::vector<std::string>::iterator j = types.begin(), je = types.end(); j != je; ++j)
		stream << *j << " ";
	std::string typesName = stream.str();
	for (int i = 0; i < 10; ++i)
	{
		Timer timer(typesName, fmap.Size());
		tester->Run(fmap.Begin(), fmap.End());
	}
}

int main(int argc, char** argv)
{
	try {
		Main(argc, argv);
		return 0;
	}
	catch (std::exception& e) {
		std::cout << "bench: " << e.what() << std::endl;
		return 1;
	}
}
