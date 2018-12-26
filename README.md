# pyimmutable

## Overview

pyimmutable offers two immutable data types for Python3.6+: a dictionary and a list. Its primary purpose is to deal with JSON-like data, i.e. data structures build from dictionaries and lists, strings, numbers, booleans and null (or None in Python). The advantage of using immutable data types for handling such data structures is that you can pass them around to other functions and classes without the danger of them being (accidentally) changed, as this would also affect your original reference to said data structure.

pyimmutable has another nice feature: two `ImmutableDict` objects with identical content are actually not two objects but one and the same object, and the same is true for `ImmutableList`. If any operation returns an `ImmutableDict` or `ImmutableList` that is identical to one that has been constructed earlier, the existing one is returned instead of a new one being constructed. This entails that e.g. two `ImmutableDict` objects, even if they represent big trees of data, can be compared using the Python `is` operation.

## Example

    from pyimmutable import ImmutableDict

    d1 = ImmutableDict()
    # d1 is an empty immutable dictionary.

    d2 = d1.set("foo", "bar")
    # d2 contains one key ("foo") whose value is "bar".
    # d1 is remains an empty immutable dictionary.

    d3 = d2.set("foo", "baz")
    # d3 now contains {"foo": "baz"}.
    # d1 and d2 remain unchanged, of course.

    d4 = ImmutableDict(foo="bar")
    # d4 also contains {"foo": "bar"}, just like d2

    d2 is d4  # True
    # d2 and d4 are actually the same object!

## Special Features for Working with JSON-like Data

`ImmutableDict` and `ImmutableList` can store any types of Python objects. Since pyimmutable is particularly useful or working with complex JSON-like objects, it is desirable to easily check if a pyimmutable object only contains immutable data that can be JSON-serialised. Therefore, pyimmutable objects have a `isImmutableJson` attribute. It is true for a `ImmutableList` if it only contains numbers, booleans, `None`, strings and `ImmutableList` and `ImmutableDict` objects whose `isImmutableJson` attribute is true, too. An `ImmutableDict`'s `isImmutableJson` is true only if it only contains strings as keys and the aforementioned types as values.

## API

Full API documentation is still missing, but here is a quick list of implemented methods:

### `ImmutableList`

-   `set(index, value)`
-   `append(value)`
-   `extend(iterable)`

### `ImmutableDict`

-   `get(key[, default])`
-   `set(key, value)`
-   `discard(key)` (does nothing if `key` is not present)
-   `pop(key)` (throws if `key` is not present)
-   `update(mapping)`
-   `keys()` (returns an iterator over the keys)
-   `values()` (returns an iterator over the values)
-   `items()` (returns an iterator over key/value tuples)

## Implementation

pyimmutable is implemented in C++17. To build pyimmutable, you need a compiler that supports C++17. A recent version of gcc or clang should be fine.

pyimmutable uses the C++ library "[immer](https://github.com/arximboldi/immer)" internally to efficiently work with immutable data. Each pyimmutable object internally stores a SHA1 hash of a representation of its contents. It uses those hashes to internally deduplicate pyimmutable objects, so that there will never be more than one pyimmutable object storing the same contents. This should not only reduce the memory footprint, but also allows for blazingly fast equality checks. In order to find out if two pyimmutable objects store the same data, you can just use the `is` operator to check if they are indeed the same object.

## Limitations / To Do

The SHA1 hashing uses object identity for most types. For some types, like strings, it instead uses the actual content to calculate the hash. In general, object identity is the correct thing to hash for all kinds of mutable objects: since their values can change during their lifetime, we can only regard two values of such types as equal when they reference the same object. For immutable types however, like strings, it is safe to regard objects as equal when their values are equal. It would also be highly impractical to treat two strings objects with the same contents as different values.

As of now, value hashing is done for strings and numbers (int, float), but not for other immutable types in Python. It should also be implemented for `tuple`, `frozenset`, `bytes`, maybe even `collections.namedtuple`.

By the way, we do not need to implement special value hashing for pyimmutable's own types: those get internally deduplicated so equal values always mean that you are looking at references to the same object. Thus object identity works just fine.

Further to do: write more documentation!
