"""Do not turn incomplete call identity evidence into parser aliases."""
import unittest
import pathlib
import tempfile

from qv_exact_aliases import select
from data_aliases import write_coff


class QvIdentity(unittest.TestCase):
    def test_unique_exact_entry(self):
        self.assertEqual(select([{'address': '00481e00'}], [], {'00481e00': ['QvGroup']}),
                         ('00481e00', 'QvGroup'))

    def test_one_unproven_reference_rejects_alias(self):
        self.assertIsNone(select([{'address': '00481e00'}], ['unmapped.cpp'],
                                 {'00481e00': ['QvGroup']}))

    def test_conflicting_call_destinations(self):
        self.assertIsNone(select([{'address': '00481e00'}, {'address': '0047e660'}], [],
                                 {'00481e00': ['QvGroup'], '0047e660': ['QvNode']}))

    def test_absent_and_ambiguous_library_owner(self):
        self.assertIsNone(select([{'address': '00481e00'}], [], {}))
        self.assertIsNone(select([{'address': '00481e00'}], [], {'00481e00': ['a', 'b']}))
        self.assertIsNone(select([], [], {'00481e00': ['QvGroup']}))

    def test_identity_and_duplicate_weak_aliases_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            target = pathlib.Path(directory) / 'alias.obj'
            with self.assertRaises(ValueError):
                write_coff([('QvGroup', 'QvGroup')], target)
            with self.assertRaises(ValueError):
                write_coff([('alias', 'a'), ('alias', 'b')], target)
            self.assertFalse(target.exists())


if __name__ == '__main__':
    unittest.main()
