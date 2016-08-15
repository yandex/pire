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


MAKO_GLOBALS = {
    "FEATURES": [
        "CaseInsensitive",
        "AndNotSupport",
    ],
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
    "SCANNERS": {
        "Scanner": ScannerSpec(),
        "NonrelocScanner": ScannerSpec(),
        "ScannerNoMask": ScannerSpec(),
        "NonrelocScannerNoMask": ScannerSpec(),
        "SimpleScanner": ScannerSpec(ignored_methods={"AcceptedRegexps", "Glue"}),
        "SlowScanner": ScannerSpec(ignored_methods={"Glue", "Size", "LettersCount"}),
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
