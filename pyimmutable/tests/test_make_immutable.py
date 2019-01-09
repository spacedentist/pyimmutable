import unittest

from pyimmutable import (
    ImmutableDict,
    ImmutableList,
    make_immutable,
    make_mutable,
)


class TestMakeImmutable(unittest.TestCase):
    example_mutable_data = {
        "foo": 1,
        "bar": {"baz": 99},
        "xxx": [None, True, []],
        "yyy": {},
        "zzz": None,
    }

    example_immutable_data = ImmutableDict(
        foo=1,
        bar=ImmutableDict(baz=99),
        xxx=ImmutableList((None, True, ImmutableList())),
        yyy=ImmutableDict(),
        zzz=None,
    )

    def test_make_immutable(self):
        i_from_m = make_immutable(self.example_mutable_data)
        self.assertTrue(i_from_m is self.example_immutable_data)
        i_from_i_from_m = make_immutable(i_from_m)
        self.assertTrue(i_from_i_from_m is i_from_m)

    def test_make_mutable(self):
        m_from_i = make_mutable(self.example_immutable_data)
        self.assertEqual(m_from_i, self.example_mutable_data)
        m_from_m_from_i = make_mutable(m_from_i)
        self.assertEqual(m_from_m_from_i, m_from_i)


if __name__ == "__main__":
    unittest.main()
