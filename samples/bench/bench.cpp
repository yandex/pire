#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <stdexcept>
#include <pire/pire.h>

class NullScanner {
public:
	typedef int State;
	typedef int Action;
    
	void Initialize(State&) const {}
	Action Next(State&, Pire::Char) const { return 0; }
	void TakeAction(State&, Action) const {}
	bool Final(const State&) const { return false; }
	unsigned StateIndex(const State&) const { return 0; }
};

template<class Scanner>
typename Scanner::State RunScanner(const Scanner& scanner, FILE* f)
{
	typename Scanner::State state;
	scanner.Initialize(state);
	Pire::Step(scanner, state, Pire::BeginMark);
    
	std::vector<char> buf(4096);
	size_t size, total = 0;
	while ((size = fread(&buf[0], 1, buf.size(), f)) != 0) {
		Pire::Run(scanner, state, &buf[0], &buf[0] + size);
		total += size;
	}
	if (ferror(f))
		throw std::runtime_error("Cannot read input stream");
    
	Pire::Step(scanner, state, Pire::EndMark);
	std::cerr << total << " bytes processed" << std::endl;
	return state;
}

class Timer {
public:
	Timer(const std::string& msg): m_msg(msg) { gettimeofday(&m_tv, 0); }
    
	~Timer()
	{
		struct timeval end;
		gettimeofday(&end, 0);
		std::cerr << m_msg << ": " << (end.tv_sec - m_tv.tv_sec) * 1000000 + (end.tv_usec - m_tv.tv_usec) << " us" << std::endl;
	}
    
private:
	std::string m_msg;
	struct timeval m_tv;
};

typedef std::vector<Pire::Fsm> Fsms;

class ITester {
public:
	virtual ~ITester() {}
	virtual void Compile(const Fsms& fsms) = 0;
	virtual void Run(FILE* f) = 0;
};

template<class Scanner>
class Tester: public ITester {
public:
	void Compile(const Fsms& fsms)
	{
		if (fsms.size() != 1)
			throw std::runtime_error("Only one regexp is allowed for this scanner");
		sc = Pire::Fsm(fsms[0]).Compile<Scanner>();
	}
    
	void Run(FILE* f)
	{
		typename Scanner::State st = RunScanner(sc, f);
		if (sc.Final(st))
			std::cerr << "Match" << std::endl;
		else
			std::cerr << "No match" << std::endl;
	}
    
private:
	Scanner sc;
};

template<>
class Tester<Pire::Scanner>: public ITester {
public:
	void Compile(const Fsms& fsms)
	{
		for (Fsms::const_iterator i = fsms.begin(), ie = fsms.end(); i != ie; ++i) {
			Pire::Scanner tsc = Pire::Fsm(*i).Compile<Pire::Scanner>();
			if (i == fsms.begin())
				tsc.Swap(sc);
			else
				sc = Pire::Scanner::Glue(sc, tsc);
		}
	}
    
	void Run(FILE* f)
	{
		Pire::Scanner::State st = RunScanner(sc, f);
		std::pair<const size_t*, const size_t*> accepted = sc.AcceptedRegexps(st);
		std::cerr << "Accepted regexps:";
		for (; accepted.first != accepted.second; ++accepted.first)
			std::cerr << " " << *accepted.first;
		std::cerr << std::endl;
	}
    
private:
	Pire::Scanner sc;
};

template<>
class Tester<NullScanner>: public ITester {
public:
	void Compile(const Fsms&) {}
	void Run(FILE* f) { RunScanner(sc, f); }
    
private:
	NullScanner sc;
};

void Main(int argc, char** argv)
{
	std::runtime_error usage("Usage: bench {--fast|--simple|--slow|--null} regexp [regexp2 [regexp3...]]");
	if (argc < 3)
		throw usage;
    
	std::auto_ptr<ITester> tester;
	std::string type(argv[1]);
	if (type == "--fast")
		tester.reset(new Tester<Pire::Scanner>);
	else if (type == "--simple")
		tester.reset(new Tester<Pire::SimpleScanner>);
	else if (type == "--slow")
		tester.reset(new Tester<Pire::SlowScanner>);
	else if (type == "--null")
		tester.reset(new Tester<NullScanner>);
	else
		throw usage;
    
	std::vector<Pire::Fsm> fsms;
	for (argc -= 2, argv += 2; argc; --argc, ++argv) {
		fsms.push_back(Pire::Lexer(std::string(*argv)).Parse().Surround());
	}
    
	tester->Compile(fsms);
    
	Timer timer(std::string(type.begin() + 2, type.end()));
	tester->Run(stdin);
}

int main(int argc, char** argv)
{
	try {
		Main(argc, argv);
		return 0;
	}
	catch (std::exception& e) {
		std::cerr << "bench: " << e.what() << std::endl;
		return 1;
	}
}
