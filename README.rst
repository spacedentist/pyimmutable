pyimmutable
===========

.. image:: https://travis-ci.org/spacedentist/pyimmutable.svg?branch=master
   :target: https://travis-ci.org/spacedentist/pyimmutable

.. image:: https://img.shields.io/badge/License-MIT-blue.svg
   :alt: License: MIT
   :target: https://opensource.org/licenses/MIT

`API documentation <https://spacedentist.github.io/pyimmutable/>`_ is hosted on GitHub pages.

.. end-header


Introduction
============

pyimmutable offers two immutable data types for Python3.6+: a mapping (similar to ``dict``, but, obviously immutable) and a sequence (like ``list``). It is implemented in C++, using the faboulous "`immer <https://github.com/arximboldi/immer>`__" library for efficiently implementing the immutable data structures.

When your Python code passes an ``ImmutableDict`` or an ``ImmutableList`` to some function, you do not have to worry that function may alter the contents of that object. A reference to such an immutable data structure is guaranteed to forever point to the same data. If you do want to change the contents of your data, you have to update the reference. So instead of ``data["foo"] = "bar"`` you will have to write ``data = data.set("foo", "bar")``. As a result, your ``data`` variable will be updated, but any other variable that referenced ``data`` before the change will keep referencing the old ``data``---the one before the update.

Immutable data structures like this allow for some interesting optimization features. For example, two ``ImmutableDict`` objects with identical content are not actually two objects but one and the same object, and the same is true for ``ImmutableList``. If any operation returns an ``ImmutableDict`` or an ``ImmutableList`` that is identical to one that has been constructed earlier, the existing one is returned instead of a new one being constructed. This entails that e.g. two ``ImmutableDict`` objects, even if they represent big trees of data, can be compared using the Python ``is`` operation.

pyimmutable has a stricter requirement for equality than the usual Python convention. In a nutshell, for pyimmutable, the integer ``3`` and the float ``3.0`` are not equal. An ``ImmutableDict`` can store different values for the keys ``3`` and ``3.0``, whereas a normal Python ``dict`` cannot. You can use objects of `any` type as keys and values, but only identical objects are considered equal. There is an exception to this rule for a small set of built-in types (`int`, `float`, `str`, `bytes`, `tuple`---all of which are immutable themselves), which are considered equal when they contain the same data.

Example
-------

>>> from pyimmutable import ImmutableDict
>>> d1 = ImmutableDict()

``d1`` is an empty immutable dictionary.

>>> d2 = d1.set("foo", "bar")

``d2`` contains one key ("foo") whose value is "bar". ``d1`` is remains an empty immutable dictionary.

>>> d3 = d2.set("fox", "baz")

``d3`` now contains two keys ("foo", "fox") with values "bar" and "baz", respectively. ``d1`` and ``d2`` remain unchanged.

>>> d4 = ImmutableDict(foo="bar", fox="baz")

``d4`` is a newly constructed ``ImmutableDict``, initialized with values ``foo`` and ``fox`` that happen to be the same as in ``d3``. In such a case, pyimmutable does not actually construct a `new` ``ImmutableDict`` for us, but returns the ``d3`` object from earlier.

>>> d3 is d4
True

This is fine, because ``ImmutableDict`` objects are immutable, and since the objects referred to by ``d3`` and ``d4`` can never be mutated and therefore will always have the same contents, ``d3`` and ``d4`` might as well refer to the same object.
