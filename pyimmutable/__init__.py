import collections.abc

from _pyimmutable import ImmutableDict, ImmutableList

__all__ = "ImmutableDict", "ImmutableList"


collections.abc.Mapping.register(ImmutableDict)
collections.abc.Sequence.register(ImmutableList)
