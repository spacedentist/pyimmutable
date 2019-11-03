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

    def test_iter(self):
        il = ImmutableList([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])

        self.assertEqual([x for x in il], list(range(10)))
        self.assertEqual([x for x in iter(il)], list(range(10)))
        self.assertEqual([x for x in il.__iter__()], list(range(10)))
        self.assertFalse([x for x in ImmutableList()])

    def test_reversed(self):
        il = ImmutableList([9, 8, 7, 6, 5, 4, 3, 2, 1, 0])
        self.assertEqual([x for x in reversed(il)], list(range(10)))
        self.assertEqual([x for x in il.__reversed__()], list(range(10)))
        self.assertEqual([x for x in iter(il.__reversed__())], list(range(10)))

    def test_count(self):
        il = ImmutableList([3, "two", 1, 3, "two", 3, 2])
        self.assertEqual(il.count(0), 0)
        self.assertEqual(il.count(1), 1)
        self.assertEqual(il.count("two"), 2)
        self.assertEqual(il.count(3), 3)
        self.assertEqual(il.count(3.0), 3)

    def test_index(self):
        with self.assertRaises(ValueError):
            ImmutableList().index(None)
        with self.assertRaises(ValueError):
            ImmutableList().index(1)
        with self.assertRaises(ValueError):
            ImmutableList([1, 2, 3]).index(4)

        self.assertEqual(ImmutableList([1, 2, 3, 4]).index(3), 2)
        self.assertEqual(ImmutableList([1, 2, 3, 4]).index(3.0), 2)


if __name__ == "__main__":
    unittest.main()
