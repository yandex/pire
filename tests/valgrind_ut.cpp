/*
 * valgrind_ut.cpp -- tests for Pire compatibility with Valgrind
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

#ifndef PIRE_ENABLE_VALGRIND_SAFE
#define PIRE_ENABLE_VALGRIND_SAFE
#endif

#include <stub/hacks.h>
#include <stub/defaults.h>
#include "stub/cppunit.h"
#include <stdexcept>
#include "common.h"

SIMPLE_UNIT_TEST_SUITE(TestPireValgrind) {

SIMPLE_UNIT_TEST(Valgrind)
{
	char* str = (char*) malloc(6);
	try {
		strcpy(str, "x\xD0\xA4yz");
		REGEXP2("x\xD0\xA4yz", "u") ACCEPTS(str);
		free(str);
	}
	catch (...) {
		free(str);
		throw;
	}
}

}
