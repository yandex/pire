#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <iostream>
#include <stdexcept>
#include <pire/pire.h>

template<class Scanner>
typename Scanner::State RunScanner(const Scanner& scanner, const char* begin, const char* end)
{
	Pire::RunHelper<Scanner> runner(scanner);
	runner.Begin().Run(begin, end).End();
	return runner.State();
}

class Timer {
public:
	Timer(const std::string& msg, size_t sz): m_msg(msg), m_sz(sz) { gettimeofday(&m_tv, 0); }
    
	~Timer()
	{
		struct timeval end;
		gettimeofday(&end, 0);
		long long usec = (end.tv_sec - m_tv.tv_sec) * 1000000 + (end.tv_usec - m_tv.tv_usec);
		float bw = m_sz *1.0 / usec;
		std::cerr << m_msg << ": " << usec << " us\t" << bw << " MB/sec" <<  std::endl;
	}
    
private:
	std::string m_msg;
	struct timeval m_tv;
	size_t m_sz;
};

typedef std::vector<Pire::Fsm> Fsms;

class ITester {
public:
	virtual ~ITester() {}
	virtual void Compile(const Fsms& fsms) = 0;
	virtual void Run(const char* begin, const char* end) = 0;
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
    
	void Run(const char* begin, const char* end)
	{
		typename Scanner::State st = RunScanner(sc, begin, end);
		if (sc.Final(st))
			std::cerr << "Match" << std::endl;
		else
			std::cerr << "No match" << std::endl;
	}
    
private:
	Scanner sc;
};

class FileMmap {
public:
	explicit FileMmap(const char *name)
		: m_fd(0)
		, m_mmap(0)
		, m_len(0)
	{
		try {
			int fd = open(name, O_RDONLY);
			if (!fd)
				throw std::runtime_error("open failed");
			m_fd = fd;
			struct stat fileStat;
			int err = fstat(m_fd, &fileStat);
			if (err)
				throw std::runtime_error("fstat failed");
			m_len = fileStat.st_size;
			m_mmap = (const char*)mmap(0, m_len, PROT_READ, MAP_PRIVATE, m_fd, 0);
			if (m_mmap == MAP_FAILED)
				throw std::runtime_error("mmap failed");
		} catch (...) {
			Close();
			throw;
		}
	}
	~FileMmap() { Close(); }
	size_t Size() const { return m_len; }
	const char* Begin() const { return m_mmap; }
	const char* End() const { return m_mmap + m_len; }

private:
	void Close()
	{
		if (m_fd)
			close(m_fd);
		m_fd = 0;
		m_mmap = 0;
		m_len = 0;
	}

	int m_fd;
	const char* m_mmap;
	size_t m_len;
};

template<class Scanner>
class MultiTester: public ITester {
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

	void Run(const char* begin, const char* end)
	{
		Pire::Scanner::State st = RunScanner(sc, begin, end);
		std::pair<const size_t*, const size_t*> accepted = sc.AcceptedRegexps(st);
		std::cerr << "Accepted regexps:";
		for (; accepted.first != accepted.second; ++accepted.first)
			std::cerr << " " << *accepted.first;
		std::cerr << std::endl;
	}
private:
	Scanner sc;
};

class MemTester: public ITester {
public:
	void Compile(const Fsms&) {}
	// Just estimates memory throughput
	void Run(const char* begin, const char* end)
	{
		size_t c = 0;
		const size_t *b = (const size_t*)begin;
		const size_t *e = (const size_t*)end;
		while (b < e) { c ^= *b++; }
		std::clog << c << std::endl;
	}
};

void Main(int argc, char** argv)
{
	std::runtime_error usage("Usage: bench {--multi|--simple|--slow|--null} file regexp [regexp2 [regexp3...]]");
	if (argc < 4)
		throw usage;
    
	std::auto_ptr<ITester> tester;
	std::string type(argv[1]);                                            
	if (type == "--multi")
		tester.reset(new MultiTester<Pire::Scanner>);
	else if (type == "--simple")
		tester.reset(new Tester<Pire::SimpleScanner>);
	else if (type == "--slow")
		tester.reset(new Tester<Pire::SlowScanner>);
	else if (type == "--null")
		tester.reset(new MemTester);
	else
		throw usage;

	const char *fname = argv[2];

	std::vector<Pire::Fsm> fsms;
	for (argc -= 3, argv += 3; argc; --argc, ++argv) {
		fsms.push_back(Pire::Lexer(std::string(*argv)).Parse().Surround());
	}
    
	tester->Compile(fsms);
    
	FileMmap fmap(fname);

	// Run the benchmark multiple times
	for (int i = 0; i < 3; ++i)
	{
		Timer timer(std::string(type.begin() + 2, type.end()), fmap.Size());
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
		std::cerr << "bench: " << e.what() << std::endl;
		return 1;
	}
}
