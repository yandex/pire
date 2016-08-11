#!/usr/bin/env python

import argparse
import os

import mako.template


MAKO_GLOBALS = {
    "features": [
        "CaseInsensitive",
        "AndNotSupport",
    ],
    "fsm_binaries": [
        ("+", "add", "const Fsm&", "Fsm"),
        ("|", "or", "const Fsm&", "Fsm"),
        ("&", "and", "const Fsm&", "Fsm"),
        ("*", "mul", "size_t", "size_t"),
    ],
    "fsm_inplace_unaries": [
        "AppendDot",
        "Surround",
        "Iterate",
        "Complement",
        "MakePrefix",
        "MakeSuffix",
        "PrependAnything",
        "AppendAnything",
        "Reverse",
        "Canonize",
        "Minimize",
    ],
    "scanners": [
        "Scanner",
        "NonrelocScanner",
        "ScannerNoMask",
        "NonrelocScannerNoMask",
    ],
    "special_chars": [
        "Epsilon",
        "BeginMark",
        "EndMark",
        "MaxCharUnaligned",
        "MaxChar",
    ],
}


def make_argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i", "--input",
        required=True,
    )
    parser.add_argument(
        "-o", "--output",
        required=True,
    )
    return parser


def main():
    options = make_argparser().parse_args()

    template = mako.template.Template(filename=os.path.abspath(options.input))
    rendered = template.render(**MAKO_GLOBALS)
    with open(options.output, "w") as out_file:
        out_file.write(rendered)


if __name__ == "__main__":
    main()
