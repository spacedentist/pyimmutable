import unittest

from pyimmutable import ImmutableDict, ImmutableList


class TestImmutableDict(unittest.TestCase):

    def verify(self, *objects):
        for x in objects:
            if type(x) in (ImmutableDict, ImmutableList):
                self.assertEqual(x.isImmutableJson, isImmutableJson(x))

    def verifyImmutable(self, *objects):
        for x in objects:
            if type(x) in (ImmutableDict, ImmutableList):
                self.assertTrue(x.isImmutableJson)
                self.assertTrue(isImmutableJson(x))

    def verifyNonImmutable(self, *objects):
        for x in objects:
            if type(x) in (ImmutableDict, ImmutableList):
                self.assertFalse(x.isImmutableJson)
                self.assertFalse(isImmutableJson(x))

    def test_dict(self):
        ch = ImmutableChecker(self)

        ch.addImmutable(ImmutableDict())
        ch.addImmutable(ImmutableDict({'a': 1}))
        ch.addImmutable(ImmutableDict({'a': 1, 'b': 2}))
        ch.addImmutable(ImmutableDict({'a': 1, 'b': 'foo'}))
        ch.addImmutable(ImmutableDict({'a': 1, 'b': None}))
        ch.addImmutable(ImmutableDict({'a': 1, 'b': True}))
        ch.addImmutable(ImmutableDict({'a': 1, 'b': 9.876}))
        ch.addNonImmutable(ImmutableDict({'a': 1, 'b': 2, 'c': Exception()}))
        ch.addImmutable(
            ImmutableDict({'a': 1, 'b': 2, 'c': Exception()})
            .discard('c'))
        ch.addImmutable(
            ImmutableDict({'a': 99, 'b': 2, 'c': Exception()})
            .discard('c'))
        ch.addNonImmutable(ImmutableDict({'a': 1, 'b': 2, True: 123}))
        ch.addImmutable(
            ImmutableDict({'a': 1, 'b': 2, True: 123})
            .discard(True))

        ch.check_all()

    def test_list(self):
        ch = ImmutableChecker(self)

        ch.addImmutable(ImmutableList())
        ch.addImmutable(ImmutableList([1]))
        ch.addImmutable(ImmutableList([1, 2]))
        ch.addImmutable(ImmutableList([1, 'foo']))
        ch.addImmutable(ImmutableList([1, None]))
        ch.addImmutable(ImmutableList([1, True]))
        ch.addImmutable(ImmutableList([1, 9.876]))
        ch.addNonImmutable(ImmutableList([1, 2, Exception()]))
        ch.addImmutable(
            ImmutableList([1, 2, Exception()])[0:2])
        ch.addImmutable(
            ImmutableList([99, 2, Exception()])[0:2])

        ch.check_all()

    def test_mixed(self):
        ch = ImmutableChecker(self)
        keys = ('', 'a', 'foobar', 'baz', '0',
                '123', 'xxx', None, False, unittest)
        values = keys + (123, 0, -1, 12.34, 567.76,
                         True, [], {}, [1, 2], {'a': 1})

        idicts = set()
        for k1 in keys:
            for v1 in values:
                i = ImmutableDict().set(k1, v1)
                idicts.add(i)
                ch.add(i)

                for k2 in keys:
                    for v2 in values:
                        i = i.set(k2, v2)
                        idicts.add(i)
                        ch.add(i)

        for i in set(idicts):
            for k in keys:
                i = i.discard(k)
                idicts.add(i)
                ch.add(i)

        ilists = set()
        for k1 in keys:
            i = ImmutableList().append(k1)
            ilists.add(i)
            ch.add(i)

            for k1 in keys:
                i = i.append(k2)
                ilists.add(i)
                ch.add(i)

        idicts = list(idicts)
        ilists = list(ilists)

        for n in range(100000):
            d1 = idicts[n * 104651 % len(idicts)]
            l1 = ilists[n * 104659 % len(ilists)]
            k1 = keys[n * 104677 % len(keys)]
            k2 = keys[n * 104681 % len(keys)]
            d2 = idicts[n * 104683 % len(idicts)]
            l2 = ilists[n * 104693 % len(ilists)]

            nl = l1.append(d1)
            ilists.append(nl)
            ch.add(nl)

            nd = d1.set(k1, l1)
            idicts.append(nd)
            ch.add(nd)

            if k2 in d1:
                nd2 = d1.discard(k2)
                idicts.append(nd2)
                ch.add(nd2)

            nl = l2.append(l1)
            ilists.append(nl)
            ch.add(nl)

            nd = d2.set(k2, d1)
            idicts.append(nd)
            ch.add(nd)

        ch.check_all()


class ImmutableChecker:
    def __init__(self, test):
        self.test = test
        self.immutables = set()
        self.nonimmutables = set()

    def addImmutable(self, x):
        try:
            self.test.assertTrue(x.isImmutableJson)
        except Exception:
            print(x)
            raise
        self.test.assertTrue(isImmutableJson(x))
        self.immutables.add(x)

    def addNonImmutable(self, x):
        try:
            self.test.assertFalse(x.isImmutableJson)
        except Exception:
            print(x)
            raise
        self.test.assertFalse(isImmutableJson(x))
        self.nonimmutables.add(x)

    def add(self, x):
        if (isImmutableJson(x)):
            return self.addImmutable(x)
        else:
            return self.addNonImmutable(x)

    def check_all(self):
        self.test.assertTrue(all(
            x.isImmutableJson for x in self.immutables))
        self.test.assertTrue(all(
            (not x.isImmutableJson) for x in self.nonimmutables))


def isImmutableJson(x):
    if x is None or type(x) in (str, bool, int, float):
        return True
    if type(x) is ImmutableDict:
        return all(
            type(k) is str and isImmutableJson(v)
            for k, v in x.items()
        )
    if type(x) is ImmutableList:
        return all(
            isImmutableJson(i)
            for i in x
        )
    return False


if __name__ == '__main__':
    unittest.main()
