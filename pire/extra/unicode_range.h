/*
 * unicode_range.h -- declaration of the UnicodeRange feature.
 *
 * Copyright (c) 2019 YANDEX LLC
 * Author: Karina Usmanova <usmanova.karin@yandex.ru>
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


#ifndef PIRE_EXTRA_UNICODE_RANGE_H
#define PIRE_EXTRA_UNICODE_RANGE_H

#include <memory>

namespace Pire {
class Feature;
namespace Features {

    /**
    * A feature which tells Pire to convert \r[...] sequences
    * to range of UTF-32 symbols and match any symbol contained in this range
    * e.g. \r[\x41-\x43] matches "a", "b" or "c"
    */
    std::unique_ptr<Feature> UnicodeRange();
}
}

#endif
