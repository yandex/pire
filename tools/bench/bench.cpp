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
#include <pire/stub/lexical_cast.h>
#include "../common/filemap.h"

#ifdef BENCH_EXTRA_ENABLED
#include <pire/extra.h>
#endif

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
		float bw = m_sz / (1.024 * 1.024 * usec);
		std::cout << m_msg << ": " << usec << " us\t" << bw << " MB/sec" <<  std::endl;
	}
    
private:
	std::string m_msg;
	long long m_tv;
	size_t m_sz;
};

typedef std::vector<std::string> Patterns;

class ITester {
public:
	enum Algorithm {
		DefaultRun,
		ShortestPrefix,
		LongestPrefix
	};

	virtual ~ITester() {}
	virtual void Prepare(Algorithm alg, const std::vector<Patterns>& patterns) = 0;
	virtual void Run(const char* begin, const char* end) = 0;
};

// Sinlge regexp scanner
template<class Scanner>
struct CompileRe {
	static Scanner Do(const Patterns& patterns, bool surround)
	{
		if (patterns.size() != 1)
			throw std::runtime_error("Only one regexp is allowed for this scanner");
		Pire::Fsm fsm = Pire::Lexer(patterns[0]).Parse();
		if (surround)
			fsm.Surround();
		return fsm.Compile<Scanner>();
	}
};

// Multi regexp scanner
template<class Relocation, class Shortcutting>
struct CompileRe< Pire::Impl::Scanner<Relocation, Shortcutting> > {
	static Pire::Impl::Scanner<Relocation, Shortcutting> Do(const Patterns& patterns, bool surround)
	{
		typedef Pire::Impl::Scanner<Relocation, Shortcutting> Sc;
		Sc sc;
		for (Patterns::const_iterator i = patterns.begin(), ie = patterns.end(); i != ie; ++i) {
			Pire::Fsm fsm = Pire::Lexer(*i).Parse();
			if (surround)
				fsm.Surround();
			Sc tsc = fsm.Compile<Sc>();
			if (i == patterns.begin())
				tsc.Swap(sc);
			else {
				sc = Sc::Glue(sc, tsc);
				if (sc.Empty()) {
					std::ostringstream msg;
					msg << "Scanner gluing failed at regexp " << *i << " - pattern too complicated";
					throw std::runtime_error(msg.str());
				}
			}
		}
		return sc;
	}
};

// Single regexp
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

// Multiple regexp
template<class Relocation, class Shortcutting>
struct PrintResult< Pire::Impl::Scanner<Relocation, Shortcutting> > {
	typedef Pire::Impl::Scanner<Relocation, Shortcutting> Scanner;

	static void Do(const Scanner& sc, typename Scanner::State st)
	{
		std::pair<const size_t*, const size_t*> accepted = sc.AcceptedRegexps(st);
		std::cout << "Accepted regexps:";
		for (; accepted.first != accepted.second; ++accepted.first)
			std::cout << " " << *accepted.first;
		std::cout << std::endl;
	}
};

// Pair result
template<class Scanner1, class Scanner2>
struct PrintResult< Pire::ScannerPair<Scanner1, Scanner2> > {
	typedef Pire::ScannerPair<Scanner1, Scanner2> Scanner;

	static void Do(const Scanner& sc, typename Scanner::State st)
	{
		std::cout << "[first] "; PrintResult<Scanner1>::Do(sc.First(), st.first);
		std::cout << "[second] "; PrintResult<Scanner2>::Do(sc.Second(), st.second);
	}
};

#ifdef BENCH_EXTRA_ENABLED
template <>
struct CompileRe<Pire::CapturingScanner> {
	static Pire::CapturingScanner Do(const Patterns& patterns, bool surround)
	{
		if (patterns.size() != 1)
			throw std::runtime_error("Only one regexp is allowed for this scanner");
		Pire::Fsm fsm = Pire::Lexer(patterns[0]).AddFeature(Pire::Features::Capture(1)).Parse();
		if (surround)
			fsm.Surround();
		return fsm.Compile<Pire::CapturingScanner>();
	}
};

template <>
struct CompileRe<Pire::CountingScanner> {
	static Pire::CountingScanner Do(const Patterns& patterns, bool /*surround*/)
	{
		Pire::CountingScanner sc;
		for (Patterns::const_iterator i = patterns.begin(), ie = patterns.end(); i != ie; ++i) {
			Pire::CountingScanner tsc = Pire::CountingScanner(
				Pire::Lexer(*i).Parse(), Pire::Lexer(".*").Parse());
			if (i == patterns.begin())
				tsc.Swap(sc);
			else {
				sc = Pire::CountingScanner::Glue(sc, tsc);
				if (sc.Empty()) {
					std::ostringstream msg;
					msg << "Scanner gluing failed at regexp " << *i << " - pattern too complicated";
					throw std::runtime_error(msg.str());
				}
			}
		}
		return sc;
	}
};

template<>
struct PrintResult<Pire::CountingScanner> {
	static void Do(const Pire::CountingScanner& sc, Pire::CountingScanner::State st)
	{
		for (size_t i = 0; i < sc.RegexpsCount(); ++i)
			std::cout << "regexp #" << i << ": " << st.Result(i) << std::endl;
	}
};

template<>
struct PrintResult<Pire::CapturingScanner> {
	static void Do(const Pire::CapturingScanner&, Pire::CapturingScanner::State st)
	{
		if (st.Captured())
			std::cout << "Match: [" << st.Begin() << ", " << st.End() << "]" << std::endl;
		else
			std::cout << "No match" << std::endl;
	}
};
#endif // BENCH_EXTRA_ENABLED

// Common implementation for all scanners
template<class Scanner>
class TesterBase: public ITester {
public:
	void Prepare(Algorithm a, const std::vector<Patterns>& patterns)
	{
		alg = a;
		Compile(patterns, alg == DefaultRun);
	}

	void Run(const char* begin, const char* end)
	{
		if (alg == DefaultRun)
			PrintResult<Scanner>::Do(sc, Pire::Runner(sc).Begin().Run(begin, end).End().State());
		else {
			const char* pos = (alg == ShortestPrefix ? 
				Pire::ShortestPrefix(sc, begin, end) :
				Pire::LongestPrefix(sc, begin, end));
			if (pos)
				std::cout << "Prefix end: " << pos - begin << std::endl;
			else
				std::cout << "No prefix" << std::endl;
		}
	}

protected:
	virtual void Compile(const std::vector<Patterns>& patterns, bool surround) = 0;

	Scanner sc;
	ITester::Algorithm alg;
};

template<class Scanner>
class Tester: public TesterBase<Scanner> {
	typedef TesterBase<Scanner> Base;

	void Compile(const std::vector<Patterns>& patterns, bool surround)
	{
		if (patterns.size() != 1)
			throw std::runtime_error("Only one set of regexps is allowed for this scanner");
		Base::sc = ::CompileRe<Scanner>::Do(patterns[0], surround);
	}
};

template<class Scanner1, class Scanner2>
class PairTester: public TesterBase< Pire::ScannerPair<Scanner1, Scanner2> > {
	typedef TesterBase< Pire::ScannerPair<Scanner1, Scanner2> > Base;

	void Compile(const std::vector<Patterns>& patterns, bool surround)
	{
		if (patterns.size() != 2)
			throw std::runtime_error("Only two sets of regexps are allowed for this scanner");
		sc1 = ::CompileRe<Scanner1>::Do(patterns[0], surround);
		sc2 = ::CompileRe<Scanner2>::Do(patterns[1], surround);
		typedef Pire::ScannerPair<Scanner1, Scanner2> Pair;
		Base::sc = Pair(sc1, sc2);
	}
private:
	Scanner1 sc1;
	Scanner2 sc2;
};


class MemTester: public ITester {
public:
	void Prepare(Algorithm, const std::vector<Patterns>&) {}
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

std::runtime_error usage(
	"Usage: bench -f file [-c repetition_count] "
	"[-a run|shortestprefix|longestprefix] "
	"-t {multi|nonreloc|multinomask|nonrelocnomask|simple|slow|null"
#ifdef BENCH_EXTRA_ENABLED
	"count|capture"
#endif
	"} regexp [regexp2 [-e regexp3...]] [-t <type> regexp4 [regexp5...]]");

ITester* CreateTester(const std::vector<std::string>& types)
{
	// TODO: is there a way to get out of this copypasting?
	if (types.size() == 1 && types[0] == "multi")
		return new Tester<Pire::Scanner>;
	else if (types.size() == 1 && types[0] == "nonreloc")
		return new Tester<Pire::NonrelocScanner>;
	else if (types.size() == 1 && types[0] == "multinomask")
		return new Tester<Pire::ScannerNoMask>;
	else if (types.size() == 1 && types[0] == "nonrelocnomask")
		return new Tester<Pire::NonrelocScannerNoMask>;
	else if (types.size() == 1 && types[0] == "simple")
		return new Tester<Pire::SimpleScanner>;
	else if (types.size() == 1 && types[0] == "slow")
		return new Tester<Pire::SlowScanner>;
	else if (types.size() == 1 && types[0] == "null")
		return new MemTester;
	else if (types.size() == 2 && types[0] == "multi" && types[1] == "multi")
		return new PairTester<Pire::Scanner, Pire::Scanner>;
	else if (types.size() == 2 && types[0] == "multinomask" && types[1] == "multinomask")
		return new PairTester<Pire::ScannerNoMask, Pire::ScannerNoMask>;
	else if (types.size() == 2 && types[0] == "multi" && types[1] == "simple")
		return new PairTester<Pire::Scanner, Pire::SimpleScanner>;
	else if (types.size() == 2 && types[0] == "multi" && types[1] == "nonreloc")
		return new PairTester<Pire::Scanner, Pire::NonrelocScanner>;
	else if (types.size() == 2 && types[0] == "simple" && types[1] == "simple")
		return new PairTester<Pire::SimpleScanner, Pire::SimpleScanner>;
	else if (types.size() == 2 && types[0] == "simple" && types[1] == "multi")
		return new PairTester<Pire::SimpleScanner, Pire::Scanner>;
	else if (types.size() == 2 && types[0] == "simple" && types[1] == "nonreloc")
		return new PairTester<Pire::SimpleScanner, Pire::NonrelocScanner>;
	else if (types.size() == 2 && types[0] == "nonreloc" && types[1] == "multi")
		return new PairTester<Pire::NonrelocScanner, Pire::Scanner>;
	else if (types.size() == 2 && types[0] == "nonreloc" && types[1] == "simple")
		return new PairTester<Pire::NonrelocScanner, Pire::SimpleScanner>;
	else if (types.size() == 2 && types[0] == "nonreloc" && types[1] == "nonreloc")
		return new PairTester<Pire::NonrelocScanner, Pire::NonrelocScanner>;
	else if (types.size() == 2 && types[0] == "nonrelocnomask" && types[1] == "nonrelocnomask")
		return new PairTester<Pire::NonrelocScannerNoMask, Pire::NonrelocScannerNoMask>;
#ifdef BENCH_EXTRA_ENABLED
	else if (types.size() == 1 && types[0] == "count")
		return new Tester<Pire::CountingScanner>;
	else if (types.size() == 1 && types[0] == "capture")
		return new Tester<Pire::CapturingScanner>;
	else if (types.size() == 2 && types[0] == "count" && types[1] == "count")
		return new PairTester<Pire::CountingScanner, Pire::CountingScanner>;
	else if (types.size() == 2 && types[0] == "capture" && types[1] == "capture")
		return new PairTester<Pire::CapturingScanner, Pire::CapturingScanner>;
#endif

	else
		throw usage;
}


void Main(int argc, char** argv)
{
	std::vector<Patterns> patterns;
	std::vector<std::string> types;
	std::string file;
	std::string algName = "run";
	int repCount = 10;
	ITester::Algorithm alg;
	for (--argc, ++argv; argc; --argc, ++argv) {
		if (!strcmp(*argv, "-t") && argc >= 2) {
			types.push_back(argv[1]);
			patterns.push_back(Patterns());
			--argc, ++argv;
		} else if (!strcmp(*argv, "-f") && argc >= 2) {
			file = argv[1];
			--argc, ++argv;
		} else if (!strcmp(*argv, "-a") && argc >= 2) {
			algName = argv[1];
			--argc, ++argv;
		} else if (!strcmp(*argv, "-c") && argc >= 2) {
			repCount = Pire::FromString<int>(argv[1]);
			--argc, ++argv;
		} else if (!strcmp(*argv, "-e") && argc >= 2) {
			if (patterns.empty())
				throw usage;
			patterns.back().push_back(argv[1]);
			--argc, ++argv;
		} else {
			if (patterns.empty())
				throw usage;
			patterns.back().push_back(*argv);
		}
	}
	if (types.empty() || file.empty() || patterns.back().empty())
		throw usage;

	if (algName == "run")
		alg = ITester::DefaultRun;
	else if (algName == "shortestprefix")
		alg = ITester::ShortestPrefix;
	else if (algName == "longestprefix")
		alg = ITester::LongestPrefix;
	else 
		throw usage;

	std::auto_ptr<ITester> tester(CreateTester(types));

	tester->Prepare(alg, patterns);
	FileMmap fmap(file.c_str());

	// Run the benchmark multiple times
	std::ostringstream stream;
	for (std::vector<std::string>::iterator j = types.begin(), je = types.end(); j != je; ++j)
		stream << *j << " ";
	std::string typesName = stream.str();
	for (int i = 0; i < repCount; ++i)
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
