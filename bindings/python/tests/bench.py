#!/usr/bin/env python2

import argparse
import cProfile
import re
import sys

import pire


class ReMatchCaller(object):
    def __init__(self, pattern):
        self.compiled_re = re.compile(pattern)

    def Call(self, line):
        return self.compiled_re.match(line)


class PireCaller(object):
    def __init__(self, pattern):
        self.scanner = pire.Scanner(pire.Lexer(pattern).Parse())


class PireMatchesCaller(PireCaller):
    def Call(self, line):
        return self.scanner.Matches(line)


class PireStateRunCaller(PireCaller):
    def Call(self, line):
        return self.scanner.InitState().Run(line).End()


CALLERS = {
    "re": ReMatchCaller,
    "pire-matches": PireMatchesCaller,
    "pire-state-run": PireStateRunCaller,
}


class TestRunner(object):
    def __init__(self, caller, lines):
        self.caller = caller
        self.lines = lines

    def Run(self):
        for line in self.lines:
            self.caller.Call(line)


def make_argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-f", "--file",
        type=argparse.FileType("r"),
        default=sys.stdin,
        help="File with lines to run on.",
    )
    parser.add_argument(
        "-m", "--multiply-input",
        type=int,
        default=1,
        help=(
            "The list of lines will be repeated (multiplied by) this number."
            "It can be used to scale the number of runs without need to create"
            "large input file."
        ),
    )
    parser.add_argument(
        "-p", "--pattern",
        required=True,
        help="Regular expression to compile.",
    )
    return parser


def main():
    options = make_argparser().parse_args()

    lines = map(str.strip, options.file.readlines())
    lines *= options.multiply_input

    for caller_name, caller_class in CALLERS.items():
        global runner
        runner = TestRunner(caller_class(options.pattern), lines)
        print caller_name
        cProfile.run("runner.Run()", sort="cumtime")


if __name__ == "__main__":
    main()
