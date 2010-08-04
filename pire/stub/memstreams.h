#ifndef PIRE_STUB_MEMSTREAMS_H_INCLUDED
#define PIRE_STUB_MEMSTREAMS_H_INCLUDED

#include <vector>
#include <iostream>
#include <memory.h>

namespace Pire {
	
	class MemBuffer {
	public:
		MemBuffer(const std::vector<char>& buf): m_buf(&buf) {}
		const char* Data() const { return m_buf->empty() ? 0 : &(*m_buf)[0]; }
		size_t Size() const { return m_buf->size(); }
		typedef std::vector<char>::const_iterator Iterator;
		Iterator Begin() const { return m_buf->begin(); }
		Iterator End() const { return m_buf->end(); }
	private:
		const std::vector<char>* m_buf;
	};
    
	typedef MemBuffer::Iterator BufferIterator; // For compatibility with Arcadia
    

	class BufferOutput: public std::ostream {
	private:
		class StreamBuf: public std::basic_streambuf<char> {
		public:
			MemBuffer Buf() const { return MemBuffer(buf); }
		protected:
			typedef std::char_traits<char> Traits;

			Traits::int_type overflow(Traits::int_type x)
			{
				buf.push_back((char) x);
				return x;
			}
			
			std::streamsize xsputn(const char* ptr, std::streamsize size)
			{
				buf.insert(buf.end(), ptr, ptr + size);
				return size;
			}
		private:
			std::vector<char> buf;
		};

		StreamBuf m_rdbuf;
		
	public:
		BufferOutput() { rdbuf(&m_rdbuf); }
		MemBuffer Buffer() const { return m_rdbuf.Buf(); }
	};
    
	class MemoryInput: public std::istream {
	private:
        
		class StreamBuf: public std::basic_streambuf<char> {
		public:
			StreamBuf(const char* data, size_t size)
			{
				char* p = const_cast<char*>(data);
				setg(p, p, p+size);
			}
			
		protected:
			std::streamsize xsgetn(char* ptr, std::streamsize size)
			{
				size = std::min(size, egptr() - gptr());
				memcpy(ptr, gptr(), size);
				gbump(size);
				return size;
			}
		};
		StreamBuf m_rdbuf;
        
	public:
		MemoryInput(const char* data, size_t size): m_rdbuf(data, size)
		{
			rdbuf(&m_rdbuf);
			exceptions(badbit | eofbit);
		}
	};	
}

#endif
