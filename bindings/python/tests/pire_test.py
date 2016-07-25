import pytest

import pire


def test_pire_provides_test_string_maker():
    pire.TestStringMaker()

def test_string_maker_makes_test_string():
    maker = pire.TestStringMaker()
    assert "test_string" == maker.MakeTestString()
