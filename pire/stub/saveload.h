#ifndef PIRE_STUB_SAVELOAD_H_INCLUDED
#define PIRE_STUB_SAVELOAD_H_INCLUDED

#include <sys/types.h>
#include <iostream>
#include <vector>
#include "../stl.h"

namespace Pire {

	namespace Impl {

		template<class Char, class Traits = std::char_traits<Char> >
		class CountingStreambuf: public std::basic_streambuf<Char, Traits> {
		public:
			CountingStreambuf(std::basic_streambuf<Char, Traits>* backend)
				: m_backend(backend)
				, m_read(0)
				, m_written(0)
			{}

			std::streamsize read() const { return m_read; }
			std::streamsize written() const { return m_written; }

		protected:
			typename Traits::int_type underflow()
			{
				typename Traits::int_type ret = m_backend->sgetc();
				if (!Traits::eq_int_type(ret, Traits::eof())) {
					m_ch = (Char) ret;
					m_read += sizeof(Char);
					setg(&m_ch, &m_ch, &m_ch+1);
				}
				return ret;
			}

			std::streamsize xsgetn(Char* data, std::streamsize len)
			{
				std::streamsize ret = m_backend->sgetn(data, len);
				m_read += ret * sizeof(Char);
				return ret;
			}

			typename Traits::int_type overflow(Char c)
			{
				typename Traits::int_type ret = m_backend->sputc(c);
				if (!Traits::eq_int_type(ret, Traits::eof()))
					m_written += sizeof(Char);
				return ret;
			}

			std::streamsize xsputn(const Char* data, std::streamsize len)
			{
				std::streamsize ret = m_backend->sputn(data, len);
				m_written += ret * sizeof(Char);
				return ret;
			}

			int sync() { return m_backend->pubsync(); }

		private:
			std::basic_streambuf<Char, Traits>* m_backend;
			std::streamsize m_read;
			std::streamsize m_written;
			Char m_ch;
		};

		template<class Char, class Traits = std::char_traits<Char> >
		class BasicAlignedInput: public std::basic_istream<Char, Traits> {
		public:
			BasicAlignedInput(std::basic_istream<Char, Traits>* backend)
				: m_streambuf(backend->rdbuf())
			{
				rdbuf(&m_streambuf);
			}

			void Align(size_t divisor = sizeof(void*))
			{
				if (m_streambuf.read() & (divisor - 1)) {
					size_t len = (divisor - (m_streambuf.read() & (divisor - 1))) / sizeof(Char);
					if (m_fill.size() < len)
						m_fill.resize(len);
					this->read(&m_fill[0], len);
				}
			}
		private:
			CountingStreambuf<Char, Traits> m_streambuf;
			std::vector<char> m_fill;
		};

		template<class Char, class Traits = std::char_traits<Char> >
		class BasicAlignedOutput: public std::basic_ostream<Char, Traits> {
		public:
			BasicAlignedOutput(std::basic_ostream<Char, Traits>* backend)
				: m_streambuf(backend->rdbuf())
			{
				rdbuf(&m_streambuf);
			}

			void Align(size_t divisor = sizeof(void*))
			{
				if (m_streambuf.written() & (divisor - 1)) {
					size_t len = (divisor - (m_streambuf.written() & (divisor - 1))) / sizeof(Char);
					if (m_fill.size() < len)
						m_fill.resize(len, char(0xCC));
					this->write(&m_fill[0], len);
				}
			}
		private:
			CountingStreambuf<Char, Traits> m_streambuf;
			std::vector<char> m_fill;
		};
	
	}

	typedef Impl::BasicAlignedInput<char> AlignedInput;
	typedef Impl::BasicAlignedOutput<char> AlignedOutput;
	template<class T>
	void SavePodType(yostream* s, const T& t)
	{
		s->write((char*) &t, sizeof(t));
	}
	
	template<class T>
	void LoadPodType(yistream* s, T& t)
	{
		s->read((char*) &t, sizeof(t));
	}
	template<class T>
	void Save(yostream* s, const T& t) { t.Save(s);}

	template<class T>
	void Load(yistream* s, T& t) { t.Load(s);}

	template<class T>
	void SaveArray(yostream* s, const T* t, size_t len)
	{
		s->write((char*) t, len * sizeof(*t));
	}

	template<class T>
	void LoadArray(yistream* s, T* t, size_t len)
	{
		s->read((char*) t, len * sizeof(*t));
	}
};

#endif
