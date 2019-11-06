import unittest

from pyimmutable import ImmutableDict, ImmutableList


class TestEquality(unittest.TestCase):
    def test_strict_equality(self):
        """
        pyimmutable has stricter equality requirements than usual.
        """

        l1 = ImmutableList([1, 2, 3])
        l2 = ImmutableList([1, 2, 3.0])

        self.assertFalse(l1 is l2)
        self.assertNotEqual(l1, l2)

        self.assertEqual(l1.count(3), 1)
        self.assertEqual(l1.count(3.0), 0)
        self.assertEqual(l2.count(3), 0)
        self.assertEqual(l2.count(3.0), 1)

        self.assertEqual(l1.index(3), 2)
        self.assertEqual(l2.index(3.0), 2)

        with self.assertRaises(ValueError):
            l1.index(3.0)

        with self.assertRaises(ValueError):
            l2.index(3)

    def test_tuple(self):
        key1 = ("x" * 10000,)
        key2 = (("x" * 9999 + "x"),)
        self.assertTrue(type(key1) is tuple)
        self.assertTrue(type(key2) is tuple)
        self.assertEqual(key1, key2)

        if key1 is key2:
            self.skipTest(
                "failed to construct two different tuple objects "
                "with same value"
            )

        d = ImmutableDict({key1: 123})
        self.assertEqual(d.get(key1), 123)
        self.assertEqual(d.get(key2), 123)
        self.assertTrue(key1 in d)
        self.assertTrue(key2 in d)

    def test_string(self):
        key1 = "x" * 10000
        key2 = "x" * 9999 + "x"
        self.assertTrue(type(key1) is str)
        self.assertTrue(type(key2) is str)
        self.assertEqual(key1, key2)

        if key1 is key2:
            self.skipTest(
                "failed to construct two different string objects "
                "with same value"
            )

        d = ImmutableDict({key1: 123})
        self.assertEqual(d.get(key1), 123)
        self.assertEqual(d.get(key2), 123)
        self.assertTrue(key1 in d)
        self.assertTrue(key2 in d)
        self.assertFalse(key1.encode("ascii") in d)

    def test_bytes(self):
        key1 = b"x" * 10000
        key2 = b"x" * 9999 + b"x"
        self.assertTrue(type(key1) is bytes)
        self.assertTrue(type(key2) is bytes)
        self.assertEqual(key1, key2)

        if key1 is key2:
            self.skipTest(
                "failed to construct two different bytes objects "
                "with same value"
            )

        d = ImmutableDict({key1: 123})
        self.assertEqual(d.get(key1), 123)
        self.assertEqual(d.get(key2), 123)
        self.assertTrue(key1 in d)
        self.assertTrue(key2 in d)
        self.assertFalse(key1.decode("ascii") in d)


if __name__ == "__main__":
    unittest.main()
