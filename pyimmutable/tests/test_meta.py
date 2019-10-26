import unittest

from pyimmutable import ImmutableDict, ImmutableList


class TestMeta(unittest.TestCase):
    def test_dict(self):
        data = ImmutableDict(a=123)
        self.assertEqual(data.meta, {})

        data.meta["been here"] = True

        self.assertTrue(ImmutableDict(a=123).meta["been here"])

    def test_list(self):
        data = ImmutableList((1, 2, 3))
        self.assertEqual(data.meta, {})

        data.meta["been here"] = True

        self.assertTrue(ImmutableList((1, 2, 3)).meta["been here"])


if __name__ == "__main__":
    unittest.main()
