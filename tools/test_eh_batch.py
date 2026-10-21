"""Graph retention tests plus rejection checks against current extracted data.

Run with flagsearch/env.sh sourced, after link_containers.py.
"""
import json
import pathlib
import tempfile
import unittest

import integration as build
from eh_batch import discover, reachable
from project_eh import parse


class RetentionTests(unittest.TestCase):
    def test_external_root_keeps_dependency_cycle(self):
        self.assertEqual(reachable({"external"}, {"external": {"info"}, "info": {"map"}, "map": {"info"}}),
                         {"external", "info", "map"})

    def test_unrooted_cycle_is_dead(self):
        self.assertEqual(reachable({"game"}, {"info": {"map"}, "map": {"info"}}), {"game"})

    def test_noncandidate_data_keeps_metadata(self):
        self.assertEqual(reachable({"game"}, {"game": {"map"}, "map": {"action"}}), {"game", "map", "action"})

    @classmethod
    def setUpClass(cls):
        cls.raw = (build.REPO / "data/data.obj").read_bytes()
        cls.symbols, cls.sections = parse(cls.raw)
        cls.inventory = json.loads((build.OUT / "eh-inventory.json").read_text())
        cls.response = (build.OUT / "link.rsp").read_text()

    def test_external_occurrence_retains_real_candidate(self):
        baseline, _ = discover(self.response, self.raw, self.symbols, self.sections, self.inventory)
        name = next(iter(baseline))
        with tempfile.TemporaryDirectory(dir=build.OUT) as folder:
            path = pathlib.Path(folder) / "reference.obj"
            path.write_bytes(name.encode() + b"\0")
            result, report = discover(self.response + '"%s"\n' % path, self.raw, self.symbols,
                                      self.sections, self.inventory)
        self.assertNotIn(name, result)
        self.assertIn(name, report["external_roots"])
        self.assertLess(len(result), len(baseline))
        self.assertTrue(set(result).issubset(baseline))

    def test_corrupt_metadata_bytes_rejected(self):
        baseline, _ = discover(self.response, self.raw, self.symbols, self.sections, self.inventory)
        si, lo, _ = next(iter(baseline.values()))
        modified = bytearray(self.raw)
        modified[self.sections[si]["offset"] + lo] ^= 1
        with self.assertRaisesRegex(ValueError, "EH bytes disagree"):
            discover(self.response, modified, self.symbols, self.sections, self.inventory)


if __name__ == "__main__":
    unittest.main()
