/*
 * pire2hyperscan_ut.cpp -- convert Pire regex to Hyperscan regex
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

#include <stub/hacks.h>
#include <stub/saveload.h>
#include <stub/utf8.h>
#include <stub/memstreams.h>
#include "stub/cppunit.h"

#include <pire.h>
#include <pire2hyperscan.h>

SIMPLE_UNIT_TEST_SUITE(TestPire2Hyperscan) {

	SIMPLE_UNIT_TEST(PireRegex2Hyperscan)
	{
		UNIT_ASSERT_EQUAL(PireRegex2Hyperscan("a.b"), "a.b");
		UNIT_ASSERT_EQUAL(PireRegex2Hyperscan("[^4]submit[^4]"), "(^|[^4])submit($|[^4])");
		UNIT_ASSERT_EQUAL(PireRegex2Hyperscan("a\\&b"), "a[&]b");
	}

	SIMPLE_UNIT_TEST(PireRegex2HyperscanThrowsTCompileException)
	{
		try {
			PireRegex2Hyperscan("a&b");
			UNIT_ASSERT(false); // must throw
		} catch (NHyperscan::TCompileException) {
			// right exception was thrown
		} catch (...) {
			UNIT_ASSERT(false); // wrong type of exception was thrown
		}
	}

}
