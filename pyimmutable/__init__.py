import collections.abc
import functools
import json

from _pyimmutable import ImmutableDict, ImmutableList

__all__ = (
    "ImmutableDict",
    "ImmutableList",
    "json_dump",
    "json_dumps",
    "json_load",
    "json_loads",
    "make_immutable",
    "make_mutable",
)


collections.abc.Mapping.register(ImmutableDict)
collections.abc.Sequence.register(ImmutableList)


def make_immutable(object):
    if isinstance(object, collections.abc.Mapping):
        return ImmutableDict(
            (make_immutable(k), make_immutable(v)) for k, v in object.items()
        )
    if isinstance(object, collections.abc.Sequence) and not isinstance(
        object, str
    ):
        return ImmutableList(make_immutable(i) for i in object)
    return object


def make_mutable(object):
    if isinstance(object, collections.abc.Mapping):
        return {make_mutable(k): make_mutable(v) for k, v in object.items()}
    if isinstance(object, collections.abc.Sequence) and not isinstance(
        object, str
    ):
        return [make_mutable(i) for i in object]
    return object


@functools.wraps(json.load)
def json_load(*args, **kwargs):
    return make_immutable(json.load(*args, **kwargs))


@functools.wraps(json.loads)
def json_loads(*args, **kwargs):
    return make_immutable(json.loads(*args, **kwargs))


@functools.wraps(json.dump)
def json_dump(object, *args, **kwargs):
    return json.dump(make_mutable(object), *args, **kwargs)


@functools.wraps(json.dumps)
def json_dumps(object, *args, **kwargs):
    return json.dumps(make_mutable(object), *args, **kwargs)
