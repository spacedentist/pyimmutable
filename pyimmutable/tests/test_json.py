import io
import json
import unittest

from pyimmutable import (
    ImmutableDict,
    ImmutableList,
    json_dump,
    json_dumps,
    json_load,
    json_loads,
)


class TestJson(unittest.TestCase):
    example_json = """
    {
        "foo": 1,
        "bar": { "baz": 99 },
        "xxx": [ null, true, []],
        "yyy": {},
        "zzz": null
    }
    """

    example_mutable_data = json.loads(example_json)

    def test_loads(self):
        d_from_j = json_loads(self.example_json)
        self.assertTrue(d_from_j is example_immutable_data())

    def test_load(self):
        d_from_j = json_load(io.StringIO(self.example_json))
        self.assertTrue(d_from_j is example_immutable_data())

    def test_dumps(self):
        j_from_d = json_dumps(example_immutable_data())
        self.assertEqual(json.loads(j_from_d), self.example_mutable_data)

    def test_dump(self):
        f = io.StringIO()
        json_dump(example_immutable_data(), f)
        j_from_d = f.getvalue()
        self.assertEqual(json.loads(j_from_d), self.example_mutable_data)


def example_immutable_data():
    return ImmutableDict(
        foo=1,
        bar=ImmutableDict(baz=99),
        xxx=ImmutableList((None, True, ImmutableList())),
        yyy=ImmutableDict(),
        zzz=None,
    )


if __name__ == "__main__":
    unittest.main()
