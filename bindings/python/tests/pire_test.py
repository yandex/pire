import pytest

import pire


SCANNER_CLASSES = [
    pire.Scanner,
    pire.ScannerNoMask,
    pire.NonrelocScanner,
    pire.NonrelocScannerNoMask,
]


def check_scanner(scanner, accepts=(), rejects=()):
    for line in accepts:
        assert scanner.Matches(line), '"%s"' % line
    for line in rejects:
        assert not scanner.Matches(line), '"%s"' % line


def check_equivalence(scanner1, scanner2, examples):
    for line in examples:
        assert scanner1.Matches(line) == scanner2.Matches(line), '"%s"' % line


@pytest.fixture(params=SCANNER_CLASSES)
def scanner_class(request):
    return request.param


@pytest.fixture()
def parse_scanner(scanner_class):
    def scanner_factory(pattern):
        lexer = pire.Lexer(pattern)
        fsm = lexer.Parse()
        return scanner_class(fsm)
    return scanner_factory


class TestFsm(object):
    def test_fsm_is_default_constructible(self):
        f = pire.Fsm()
        assert 1 == f.Size()

    def test_fsm_can_be_made_false(self):
        f = pire.Fsm.MakeFalse()
        assert 1 == f.Size()

    def test_default_fsm_compiles_to_default_scanner(self):
        scanner = pire.Fsm().Compile()
        assert pire.Scanner == type(scanner)

    def test_fsm_compiles_to_scanner_of_choice(self, scanner_class):
        assert scanner_class == type(pire.Fsm().Compile(scanner_class))

    def test_fsm_is_copy_constructible(self):
        fsm = pire.Fsm().Append("ab")
        fsm_copy = pire.Fsm(fsm)
        assert fsm_copy is not fsm
        check_equivalence(
            fsm.Compile(),
            fsm_copy.Compile(),
            ["", "a", "ab", "ab-", "-"]
        )

        fsm.Append("c")
        assert not fsm_copy.Compile().Matches("abc")


class TestLexer(object):
    def test_lexer_default_constructible(self):
        lexer = pire.Lexer()
        assert pire.Fsm == type(lexer.Parse())

    def test_lexer_cannot_be_constructed_with_wrong_argument(self):
        pytest.raises(TypeError, pire.Lexer, 42)

    def test_lexer_parses_valid_regexp_right(self, parse_scanner):
        check_scanner(
            parse_scanner(""),
            accepts=[""],
            rejects=["some"],
        )
        check_scanner(
            parse_scanner("a|b|c"),
            accepts=["a", "b", "c"],
            rejects=["", "ab", "ac", "bc", "aa", "bb", "cc"],
        )

    def test_lexer_raises_on_parsing_invalid_regexp(self):
        pytest.raises(Exception, pire.Lexer("[ab").Parse)


class TestScanner(object):
    def test_scanner_inherits_from_base_scanner(self, scanner_class):
        assert issubclass(scanner_class, pire.BaseScanner)

    def test_scanner_is_default_constructible(self, scanner_class):
        scanner = scanner_class()
        assert scanner.Empty()
        assert 1 == scanner.Size()
        check_scanner(scanner, rejects=["", "some"])

    def test_scanner_raises_when_matching_not_string_but_stays_valid(self, parse_scanner):
        scanner = parse_scanner("s(om)*e")
        for invalid_input in [None, False, True, 0, 42]:
            pytest.raises(Exception, scanner.Matches, invalid_input)
        check_scanner(scanner, accepts=["se", "somome"], rejects=["", "s"])
