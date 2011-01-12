#ifndef PIRE_EASY_H_INCLUDED
#define PIRE_EASY_H_INCLUDED

#include "pire.h"
#include "stub/stl.h"

namespace Pire {
	
template<class Arg> class Option;

class Options {
public:
	Options(): m_encoding(&Pire::Encodings::Latin1()) {}
	~Options() { Clear(); }
	
	void Add(const Encoding& encoding) { m_encoding = &encoding; }
	void Add(Feature* feature) { m_features.push_back(feature); }
	
	struct Proxy {
		Options* const o;
		/*implicit*/ Proxy(Options* opts): o(opts) {}
	};
	operator Proxy() { return Proxy(this); }
	
	Options(Options& o): m_encoding(o.m_encoding) { m_features.swap(o.m_features); }
	Options& operator = (Options& o) { m_encoding = o.m_encoding; m_features = o.m_features; o.Clear(); return *this; }
	
	Options(Proxy p): m_encoding(p.o->m_encoding) { m_features.swap(p.o->m_features); }
	Options& operator = (Proxy p) { m_encoding = p.o->m_encoding; m_features = p.o->m_features; p.o->Clear(); return *this; }
	
	void Apply(Lexer& lexer)
	{
		lexer.SetEncoding(*m_encoding);
		for (yvector<Feature*>::iterator i = m_features.begin(), ie = m_features.end(); i != ie; ++i) {
			lexer.AddFeature(*i);
			*i = 0;
		}
		m_features.clear();
	}
	
	template<class ArgT>
	/*implicit*/ Options(const Option<ArgT>& option);

private:
	const Encoding* m_encoding;
	yvector<Feature*> m_features;
	
	void Clear()
	{
		for (yvector<Feature*>::iterator i = m_features.begin(), ie = m_features.end(); i != ie; ++i) {
			if (*i)
				(*i)->Destroy();
		}
		m_features.clear();
	}
};

template<class Arg>
class Option {
public:
	typedef Arg (*Ctor)();

	Option(Ctor ctor): m_ctor(ctor) {}

	friend Options operator | (Options::Proxy options, const Option<Arg>& self)
	{
		Options ret(options);
		ret.Add((*self.m_ctor)());
		return ret;
	}
	
	template<class Arg2>
	friend Options operator | (const Option<Arg2>& a, const Option<Arg>& b)
	{
		return Options() | a | b;
	}

private:
	Ctor m_ctor;
};


extern const Option<const Encoding&> UTF8;
extern const Option<const Encoding&> LATIN1;

extern const Option<Feature*> I;
extern const Option<Feature*> ANDNOT;


class Regexp {
public:
	explicit Regexp(const ystring& pattern, Options options = Options())
	{
		Lexer l(pattern);
		options.Apply(l);
		Init(l.Parse());
	}
	
	explicit Regexp(const char* pattern, Options options = Options())
	{
		Lexer l(pattern);
		options.Apply(l);
		Init(l.Parse());
	}

	template<class Arg>
	Regexp(const ystring& pattern, Option<Arg> option)
	{
		Lexer l(pattern);
		(Options() | option).Apply(l);
		Init(l.Parse());
	}
	
	template<class Arg>
	Regexp(const char* pattern, Option<Arg> option)
	{
		Lexer l(pattern);
		(Options() | option).Apply(l);
		Init(l.Parse());
	}

	explicit Regexp(Scanner sc): m_scanner(sc) {}
	explicit Regexp(SlowScanner ssc): m_slow(ssc) {}
	
	bool Matches(const char* begin, const char* end) const
	{
		if (!m_scanner.Empty())
			return Runner(m_scanner).Begin().Run(begin, end).End();
		else
			return Runner(m_slow).Begin().Run(begin, end).End();
	}
	
	bool Matches(const char* str) const { return Matches(str, str + strlen(str)); }
	
	bool Matches(const ystring& str) const
	{
		static const char c = 0;
		return str.empty() ? Matches(&c, &c) : Matches(&str[0], &str[0] + str.size());
	}
	
	/// A helper class allowing '==~' operator for regexps
	class MatchProxy {
	public:
		MatchProxy(const Regexp& re): m_re(&re) {}
		friend bool operator == (const char* str, const MatchProxy& re)    { return re.m_re->Matches(str); }
		friend bool operator == (const ystring& str, const MatchProxy& re) { return re.m_re->Matches(str); }

	private:
		const Regexp* m_re;
	};
	MatchProxy operator ~() const { return MatchProxy(*this); }
		
private:
	Scanner m_scanner;
	SlowScanner m_slow;
	
	void Init(Pire::Fsm fsm)
	{
		fsm.Surround();
		if (fsm.Determine())
			m_scanner = fsm.Compile<Scanner>();
		else
			m_slow = fsm.Compile<SlowScanner>();
	}
	
};

};

#endif
