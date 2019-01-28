import unittest
import weakref

from pyimmutable import ImmutableDict, ImmutableList


class TestWeakReference(unittest.TestCase):
    def test_dict_weakref(self):
        # Create one ImmutableDict
        self.assertEqual(ImmutableDict._get_instance_count(), 0)
        d = ImmutableDict(foo=1, bar=2)
        self.assertEqual(ImmutableDict._get_instance_count(), 1)

        # Store in a weakref.WeakValueDictionary
        wvd = weakref.WeakValueDictionary()
        wvd[1] = d
        self.assertEqual(ImmutableDict._get_instance_count(), 1)
        self.assertEqual(len(wvd), 1)
        self.assertTrue(wvd[1] is d)

        # Release the only reference to the ImmutableDict
        d = None
        self.assertEqual(ImmutableDict._get_instance_count(), 0)
        # Check that it is no longer in wkd
        self.assertEqual(len(wvd), 0)
        with self.assertRaises(KeyError):
            wvd[1]

        # Create a new ImmutableDict, check that it not magically appears in
        # wkd
        d = ImmutableDict(foo=1, bar=2)
        self.assertEqual(ImmutableDict._get_instance_count(), 1)
        self.assertEqual(len(wvd), 0)
        with self.assertRaises(KeyError):
            wvd[1]

    def test_list_weakref(self):
        # Create one ImmutableList
        self.assertEqual(ImmutableList._get_instance_count(), 0)
        d = ImmutableList([1, 2])
        self.assertEqual(ImmutableList._get_instance_count(), 1)

        # Store in a weakref.WeakValueListionary
        wvd = weakref.WeakValueDictionary()
        wvd[1] = d
        self.assertEqual(ImmutableList._get_instance_count(), 1)
        self.assertEqual(len(wvd), 1)
        self.assertTrue(wvd[1] is d)

        # Release the only reference to the ImmutableList
        d = None
        self.assertEqual(ImmutableList._get_instance_count(), 0)
        # Check that it is no longer in wkd
        self.assertEqual(len(wvd), 0)
        with self.assertRaises(KeyError):
            wvd[1]

        # Create a new ImmutableList, check that it not magically appears in
        # wkd
        d = ImmutableList([1, 2])
        self.assertEqual(ImmutableList._get_instance_count(), 1)
        self.assertEqual(len(wvd), 0)
        with self.assertRaises(KeyError):
            wvd[1]


if __name__ == "__main__":
    unittest.main()
