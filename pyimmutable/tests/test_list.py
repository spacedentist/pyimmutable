import unittest

from pyimmutable import ImmutableList


class TestImmutableDict(unittest.TestCase):
    def test_append_get_set(self):
        l0 = ImmutableList()
        self.assertEqual(len(l0), 0)
        self.assertEqual(list(l0), [])
        l1 = l0.append(999)
        self.assertEqual(len(l1), 1)
        self.assertEqual(l1[0], 999)
        self.assertEqual(list(l1), [999])
        l2 = l1.append("foobar")
        self.assertEqual(len(l2), 2)
        self.assertEqual(l2[0], 999)
        self.assertEqual(l2[1], "foobar")
        self.assertEqual(list(l2), [999, "foobar"])
        l3 = l2.set(0, "baz")
        self.assertEqual(len(l3), 2)
        self.assertEqual(l3[0], "baz")
        self.assertEqual(l3[1], "foobar")
        self.assertEqual(list(l3), ["baz", "foobar"])

    def test_concat_extend(self):
        lists = [[], [0], ["foo", "bar"], [False, True, None, 0.123]]

        for l1 in lists:
            for l2 in lists:
                il1 = ImmutableList(l1)
                il2 = ImmutableList(l2)
                self.assertTrue(ImmutableList(l1 + l2) is il1 + il2)
                self.assertTrue(ImmutableList(l1 + l2) is il1.extend(il2))
                self.assertTrue(ImmutableList(l1 + l2) is il1.extend(l2))


if __name__ == "__main__":
    unittest.main()
