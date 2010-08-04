#ifndef PIRE_VBITSET_H
#define PIRE_VBITSET_H


#include "stub/stl.h"
#include <assert.h>
#include <string.h>

namespace Pire {

#ifdef _DEBUG
#define VBITSET_CHECK_SIZE(x) CheckSize(x)
#else
#define VBITSET_CHECK_SIZE(x) x
#endif

/// A bitset with variable width
class BitSet {
public:
	typedef size_t value_type;
	typedef size_t* pointer;
	typedef size_t& reference;
	typedef const size_t& const_reference;

	class const_iterator;

	BitSet()
		: m_data(1, 1)
	{
	}
	BitSet(size_t size)
		: m_data(RoundUp(size + 1) + 1)
		, m_size(size)
	{
		m_data[RoundDown(size)] |= (1U << Remainder(size));
	}

	void Swap(BitSet& s)
	{
		m_data.swap(s.m_data);
		DoSwap(m_size, s.m_size);
	}

	/// Sets the specified bit to 1.
	void Set(size_t pos) {
		m_data[RoundDown(VBITSET_CHECK_SIZE(pos))] |= (1U << Remainder(pos));
	}

	/// Resets the specified bit to 0.
	void Reset(size_t pos) {
		m_data[RoundDown(VBITSET_CHECK_SIZE(pos))] &= ~(1U << Remainder(pos));
	}

	/// Checks whether the specified bit is set to 1.
	bool Test(size_t pos) const {
		return (m_data[RoundDown(VBITSET_CHECK_SIZE(pos))] & (1U << Remainder(pos))) != 0;
	}

	size_t Size() const {
		return m_size;
	}

	void Resize(size_t newsize)
	{
		m_data.resize(RoundUp(newsize + 1));
		if (Remainder(newsize) && !m_data.empty())
			m_data[m_data.size() - 1] &= ((1U << Remainder(newsize)) - 1); // Clear tail
		m_data[RoundDown(newsize)] |= (1U << Remainder(newsize));
	}

	/// Resets all bits to 0.
	void Clear() { memset(&m_data[0], 0, m_data.size() * sizeof(ContainerType)); }

private:
	typedef unsigned char ContainerType;
	static const size_t ItemSize = sizeof(ContainerType) * 8;
	yvector<ContainerType> m_data;
	size_t m_size;

	static size_t RoundUp(size_t x)   { return x / ItemSize + ((x % ItemSize) ? 1 : 0); }
	static size_t RoundDown(size_t x) { return x / ItemSize; }
	static size_t Remainder(size_t x) { return x % ItemSize; }

#ifdef _DEBUG
	size_t CheckSize(size_t size) const
	{
		if (size < m_size)
			return size;
		else
			throw Error("BitSet: subscript out of range");
	}
#endif
};

}

#endif
