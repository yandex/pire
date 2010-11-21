/*
 * memstreams.h -- a wrapper providing istream/ostream interface
 *                 for memory ranges.
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
    

	class BufferOutputBase {
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

	protected:
		StreamBuf m_rdbuf;
		
	public:
		BufferOutputBase() { }
		MemBuffer Buffer() const { return m_rdbuf.Buf(); }
	};

	class BufferOutput: public BufferOutputBase, public std::ostream {
	public:
		BufferOutput() : std::ostream(&m_rdbuf) {}
	};
    
	class MemoryInputBase {
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
				size = std::min<std::streamsize>(size, egptr() - gptr());
				memcpy(ptr, gptr(), size);
				gbump(size);
				return size;
			}
		};

	protected:
		StreamBuf m_rdbuf;
        
	public:
		MemoryInputBase(const char* data, size_t size): m_rdbuf(data, size) {}
	};	

	class MemoryInput : protected MemoryInputBase, public std::istream {
	public:
		MemoryInput(const char* data, size_t size)
			: MemoryInputBase(data, size)
			, std::istream(&m_rdbuf)
		{
			exceptions(badbit | eofbit);
		}
	};
}

#endif
