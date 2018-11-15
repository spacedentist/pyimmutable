import collections
import gc
import random
import sys
import unittest

from pyimmutable import ImmutableDict


class StressTestImmutableDict(unittest.TestCase):
    def test_stress(self):
        r = random.Random(123)

        class X:
            pass
        x = X()

        objects = [x, x, True, False, None,
                   "\uf111", -4.5, -4.501, -4.502, 4.5]
        objects.extend(range(-50, 50))
        objects.extend(str(r.random()) for i in range(100))
        objects.append(unittest)

        dicts = list(
            (ImmutableDict(), {})
            for i in range(10)
        )

        for i in range(1000000):
            # Mutate actions
            idx = random.randrange(0, len(dicts))
            idict, pydict = dicts[idx]

            action = random.randint(0, 1)
            if action == 0:
                key, value = random.choice(objects), random.choice(objects)
                idict = idict.set(key, value)
                pydict[key] = value
            elif action == 1:
                key = random.choice(objects)
                idict = idict.discard(key)
                pydict.pop(key, None)

            dicts[idx] = idict, pydict

            # Verify actions
            idict, pydict = random.choice(dicts)
            action = random.randint(0, 13)
            if action < 5:
                key = random.choice(objects)
                self.assertEqual(idict.get(key, -999), pydict.get(key, -999))
            elif action < 10:
                key = random.choice(objects)
                try:
                    value = idict[key]
                except KeyError:
                    value = KeyError
                self.assertEqual(value, pydict.get(key, KeyError))
            elif action == 10:
                self.assertTrue(compareUnsortedUniqueSequences(
                    idict, pydict))
            elif action == 11:
                self.assertTrue(compareUnsortedUniqueSequences(
                    idict.keys(), pydict))
            elif action == 12:
                self.assertTrue(compareUnsortedSequences(
                    idict.values(), pydict.values()))
            elif action == 13:
                self.assertTrue(compareUnsortedUniqueSequences(
                    idict.items(), pydict.items()))

        for idict, pydict in dicts:
            self.assertEqual(dict(idict), pydict)
            self.assertTrue(idict is ImmutableDict(pydict))

        # check for memory leaks
        del X, action, dicts, i, idict, idx, key, objects, r, pydict, value
        gc.collect()
        self.assertEqual(ImmutableDict._get_instance_count(), 0)
        # FIXME: self.assertLessEqual(sys.getrefcount(x), 4)
        # The above fails badly, looks like there is a memory leak


def compareUnsortedSequences(a, b):
    la = list(a)
    lb = list(b)

    if len(la) != len(lb):
        return False

    da = collections.defaultdict(int)
    db = collections.defaultdict(int)

    for i in la:
        da[i] += 1
    for i in lb:
        db[i] += 1

    return da == db


def compareUnsortedUniqueSequences(a, b):
    la = list(a)
    lb = list(b)
    sa = set(la)
    sb = set(lb)
    return len(la) == len(lb) == len(sa) == len(sb) and sa == sb


if __name__ == '__main__':
    unittest.main()
