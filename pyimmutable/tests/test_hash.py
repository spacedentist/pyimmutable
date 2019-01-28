import unittest

from pyimmutable import ImmutableList


def make_test_data():
    return [tuple([0]), tuple([0, 1, 2]), " ".join("some text string".split())]


class TestHash(unittest.TestCase):
    def test_hash(self):
        data1 = make_test_data()
        data2 = make_test_data()

        if all(x is y for x, y in zip(data1, data2)):
            # If the members of data1 and data2 are pairwise identical (they
            # are the same objects), then this test is not meaningful.
            # This means that constructing the same tuple or string twice
            # yields the same object. This was not the case with the Python
            # interpreter used during development.
            raise unittest.SkipTest("Test data sets contain identical objects")

        l1 = ImmutableList(data1)
        l2 = ImmutableList(data2)
        self.assertTrue(l1 is l2)


if __name__ == "__main__":
    unittest.main()
