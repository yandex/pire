/*
 * validator.cpp -- a simple example of using regexp inlining
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

#include <iostream>
#include <iomanip>
#include <string>
#include <pire/pire.h>

int main()
{
	std::string email;
	std::cout << "Enter email: " << std::flush;
	while (std::cin >> email) {
		if (!Pire::Runner(PIRE_REGEXP("^[a-z0-9\\.\\-_]+@([a-z0-9\\-]+\\.)+[a-z]{2,}$", "")).Begin().Run(email).End())
			std::cout << "Invalid email, try once more" << std::endl;
		std::cout << "Enter email: " << std::flush;
	}
	std::cout << std::endl;
	return 0;
}
