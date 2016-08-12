#!/usr/bin/env python

import argparse
import os
import sys

import mako.template


class ScannerSpec(object):
    def __init__(self, state_t="size_t", extra_methods=(), ignored_methods=()):
        self.state_t = state_t
        self.extra_methods = extra_methods
        self.ignored_methods = ignored_methods


class OptionSpec(object):
    def __init__(self, cpp_getter, letter=""):
        assert len(letter) <= 1
        self.cpp_getter = cpp_getter
        self.letter = letter


MAKO_GLOBALS = {
    "FSM_BINARIES": [
        ("+", "add", "const Fsm&", "Fsm"),
        ("|", "or", "const Fsm&", "Fsm"),
        ("&", "and", "const Fsm&", "Fsm"),
        ("*", "mul", "size_t", "size_t"),
    ],
    "FSM_INPLACE_UNARIES": [
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
    "OPTIONS": {
        "LATIN1": OptionSpec("Pire::Encodings::Latin1()", "l"),
        "UTF8": OptionSpec("Pire::Encodings::Utf8()", "u"),
        "I": OptionSpec("Pire::Features::CaseInsensitive()", "i"),
        "ANDNOT": OptionSpec("Pire::Features::AndNotSupport()", "a"),
    },
    "SCANNERS": {
        "Scanner": ScannerSpec(),
        "NonrelocScanner": ScannerSpec(),
        "ScannerNoMask": ScannerSpec(),
        "NonrelocScannerNoMask": ScannerSpec(),
        "SimpleScanner": ScannerSpec(ignored_methods={"AcceptedRegexps", "Glue"}),
        "SlowScanner": ScannerSpec(
            state_t="yvector[size_t]",
            ignored_methods={"Glue", "Size", "LettersCount"}
        ),
    },
    "SPECIAL_CHARS": [
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

    try:
        rendered = template.render(**MAKO_GLOBALS)
    except:
        traceback = mako.exceptions.RichTraceback()
        for (filename, lineno, function, line) in traceback.traceback:
            print "  File %s, line %s, in %s" % (filename, lineno, function)
            print "    %s" % line
        print "%s: %s" % (str(traceback.error.__class__.__name__), traceback.error)
        sys.exit(1)

    with open(options.output, "w") as out_file:
        out_file.write(rendered)


if __name__ == "__main__":
    main()
