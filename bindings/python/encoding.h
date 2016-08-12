#pragma once

#include "pire/encoding.h"


namespace PireBinding {
Pire::yvector<Pire::wchar32> Utf8ToUcs4(const char* begin, const char* end) {
    Pire::yvector<Pire::wchar32> ucs4;
    Pire::Encodings::Utf8().FromLocal(begin, end, std::back_inserter(ucs4));
    return ucs4;
}
}
