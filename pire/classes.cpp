#include "stl.h"
#include "stub/singleton.h"
#include "stub/noncopyable.h"
#include "stub/utf8.h"
#include "re_lexer.h"

namespace Pire {

namespace {

	class CharClassesTable: private NonCopyable {
	private:
		class CharClass {
		public:
			CharClass() {}
			explicit CharClass(wchar32 ch) { m_bounds.push_back(std::make_pair(ch, ch)); }
			CharClass(wchar32 lower, wchar32 upper) { m_bounds.push_back(std::make_pair(lower, upper)); }

			CharClass& operator |= (const CharClass& cc)
			{
				std::copy(cc.m_bounds.begin(), cc.m_bounds.end(), std::back_inserter(m_bounds));
				return *this;
			}

			CharClass  operator |  (const CharClass& cc) const
			{
				CharClass r(*this);
				r |= cc;
				return r;
			}

			std::set<wchar32> ToSet() const
			{
				std::set<wchar32> ret;
				for (std::vector<std::pair<wchar32, wchar32> >::const_iterator it = m_bounds.begin(), ie = m_bounds.end(); it != ie; ++it)
					for (wchar32 c = it->first; c <= it->second; ++c)
						ret.insert(c);
				return ret;
			}

		private:
			std::vector<std::pair<wchar32, wchar32> > m_bounds;
		};

	public:
		bool Has(wchar32 wc) const
		{
			return (m_classes.find(to_lower(wc & ~ControlMask)) != m_classes.end());
		}

		std::set<wchar32> Get(wchar32 wc) const
		{
			std::map<wchar32, CharClass>::const_iterator it = m_classes.find(to_lower(wc & ~ControlMask));
			if (it == m_classes.end())
				throw Error("Unknown character class");
			return it->second.ToSet();
		}

		CharClassesTable()
		{
			m_classes['l'] = CharClass('A', 'Z') | CharClass('a', 'z');
			m_classes['c']
				= CharClass(0x0410, 0x044F) // Russian capital A to Russan capital YA, Russian small A to Russian small YA
				| CharClass(0x0401)         // Russian capital Yo
				| CharClass(0x0451)         // Russian small Yo
				;

			m_classes['w'] = m_classes['l'] | m_classes['c'];
			m_classes['d'] = CharClass('0', '9');
			m_classes['s']
				= CharClass(' ') | CharClass('\t') | CharClass('\r') | CharClass('\n')
				| CharClass(0x00A0)         // Non-breaking space
				;

			// A few special classes which do not have any negation
			m_classes['n'] = CharClass('\n');
			m_classes['r'] = CharClass('\r');
			m_classes['t'] = CharClass('\t');
		}

		std::map<wchar32, CharClass> m_classes;
	};

	class CharClassesImpl: public Feature {
	public:
		CharClassesImpl(): m_table(Singleton<CharClassesTable>()) {}
		int Priority() const { return 10; }

		void Alter(Term& t)
		{
			if (t.Value().IsA<Term::CharacterRange>()) {
				const Term::CharacterRange& range = t.Value().As<Term::CharacterRange>();
				typedef Term::CharacterRange::first_type CharSet;
				const CharSet& old = range.first;
				CharSet altered;
				bool pos = false;
				bool neg = false;
				for (CharSet::const_iterator i = old.begin(), ie = old.end(); i != ie; ++i)
					if (i->size() == 1 && ((*i)[0] & ControlMask) == Control && m_table->Has((*i)[0])) {
						if (is_upper((*i)[0] & ~ControlMask))
							neg = true;
						else
							pos = true;

						std::set<wchar32> klass = m_table->Get((*i)[0]);
						for (std::set<wchar32>::iterator j = klass.begin(), je = klass.end(); j != je; ++j)
							altered.insert(Term::String(1, *j));
					} else
						altered.insert(*i);

				if (neg && (pos || range.second))
					Error("Positive and negative character ranges mixed");
				t = Term(t.Type(), Term::CharacterRange(altered, neg || range.second));
			}
		}

	private:
		CharClassesTable* m_table;
	};

}

namespace Features {
	Feature* CharClasses() { return new CharClassesImpl; }
}

}

