import unittest

from pyimmutable import ImmutableDict


class TestImmutableDict(unittest.TestCase):

    def test_getset(self):
        d = ImmutableDict()
        with self.assertRaises(KeyError):
            d['foo']
        self.assertEqual(d.get('foo'), None)
        e = d.set('foo', 'bar')
        with self.assertRaises(KeyError):
            d['foo']
        self.assertEqual(e.get('foo'), 'bar')

    def test_equal(self):
        d = ImmutableDict().set('foo', 1).set('bar', 2)
        e = ImmutableDict().set('bar', 2).set('foo', 1)
        self.assertEqual(d, e)
        self.assertTrue(d is e)

    def test_iter(self):
        d = ImmutableDict().set('foo', 1).set('bar', 2)
        self.assertEqual(set(d), {'foo', 'bar'})
        self.assertEqual(set(d.keys()), {'foo', 'bar'})
        self.assertEqual(set(d.values()), {1, 2})
        self.assertEqual(set(d.items()), {('foo', 1), ('bar', 2)})

    def test_len(self):
        self.assertEqual(len(
            ImmutableDict()
        ), 0)
        self.assertEqual(len(
            ImmutableDict().set('foo', 1)
        ), 1)
        self.assertEqual(len(
            ImmutableDict().set('foo', 1).set('bar', 2)
        ), 2)
        self.assertEqual(len(
            ImmutableDict().set('foo', 1).set('bar', 2).set('foo', 99)
        ), 2)

    def test_convertToDict(self):
        pydict = {'a': 1, 'b': None, 'c': True, False: 5.678}
        d = ImmutableDict()
        for key, val in pydict.items():
            d = d.set(key, val)
        self.assertEqual(dict(d), pydict)

    def test_constructFromDict(self):
        pydict = {'a': 1, 'b': None, 'c': True, False: 5.678}
        d = ImmutableDict(pydict)
        self.assertEqual(set(d.items()), set(pydict.items()))


if __name__ == '__main__':
    unittest.main()
