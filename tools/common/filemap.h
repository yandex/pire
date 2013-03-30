/*
 * filemap.h --
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

#ifndef PIRE_TOOLS_COMMON_H_INCLUDED
#define PIRE_TOOLS_COMMON_H_INCLUDED

#include <stdexcept>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

class FileMmap {
public:
	explicit FileMmap(const char *name)
		: m_fd(0)
		, m_mmap(0)
		, m_len(0)
	{
		try {
			int fd = open(name, O_RDONLY);
			if (fd == -1)
				throw std::runtime_error(std::string("open failed for ") + name + ": " + strerror(errno));
			m_fd = fd;
			struct stat fileStat;
			int err = fstat(m_fd, &fileStat);
			if (err)
				throw std::runtime_error(std::string("fstat failed for") + name + ": " + strerror(errno));
			m_len = fileStat.st_size;
			const char* addr = (const char*)mmap(0, m_len, PROT_READ, MAP_PRIVATE, m_fd, 0);
			if (addr == MAP_FAILED)
				throw std::runtime_error(std::string("mmap failed for ") + name + ": " + strerror(errno));
			m_mmap = addr;
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
		if (m_mmap)
			munmap((void*)m_mmap, m_len);
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

#else // _WIN32
#include <windows.h>

class FileMmap {
public:
	explicit FileMmap(const char *name)
		: m_fd(0)
		, m_fm(0)
		, m_mmap(0)
		, m_len(0)
	{
		try {
			HANDLE fd = CreateFileA(
				name,
				GENERIC_READ,
				FILE_SHARE_READ,
				0,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0);
			if (fd == INVALID_HANDLE_VALUE)
				throw FileError(name, "CreateFileA");
			m_fd = fd;

			if (!GetFileSizeEx(m_fd, (PLARGE_INTEGER)&m_len))
				throw FileError(name, "GetFileSizeEx");

			HANDLE fm = CreateFileMapping(
				m_fd,
				NULL,
				PAGE_READONLY,
				(DWORD)(m_len >> 32),
				(DWORD)(m_len & 0xFFFFFFFF),
				NULL);
			if (fm == INVALID_HANDLE_VALUE)
				throw FileError(name, "CreateFileMapping");
			m_fm = fm;

			void* addr = MapViewOfFile(m_fm, FILE_MAP_READ, 0, 0, m_len);
			if (addr == 0)
				throw FileError(name, "MapViewOfFile");
			m_mmap = (const char*)addr;
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
	std::runtime_error FileError(const std::string& name, const std::string& operation)
	{
		return std::runtime_error(operation + " failed for " + 
			name + " with error: " + Pire::ToString(GetLastError()));
	}

	void Close()
	{
		if (m_mmap)
			UnmapViewOfFile(m_mmap);
		if (m_fm)
			CloseHandle(m_fm);
		if (m_fd)
			CloseHandle(m_fd);
		m_fd = 0;
		m_fm = 0;
		m_mmap = 0;
		m_len = 0;
	}

	HANDLE m_fd;
	HANDLE m_fm;
	const char* m_mmap;
	unsigned long long m_len;
};

#endif // _WIN32

#endif // PIRE_TOOLS_COMMON_H_INCLUDED

