#pragma once

#include "pire/pire.h"
#include "pire/easy.h"


namespace PireBinding {
enum OptionFlag {
    % for option in OPTIONS:
    ${option},
    % endfor
};

typedef Pire::yset<OptionFlag> FlagSet;

inline Pire::yauto_ptr<Pire::Options> ConvertFlagSetToOptions(const FlagSet& options) {
    Pire::yauto_ptr<Pire::Options> converted(new Pire::Options());
    % for option, spec in OPTIONS.items():
    if (options.count(${option})) {
        converted->Add(${spec.cpp_getter});
    }
    % endfor
    return converted;
}
}
// vim: ft=cpp
